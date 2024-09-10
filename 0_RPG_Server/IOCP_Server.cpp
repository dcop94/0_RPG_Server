// IOCP Network Server Code
// 2024.09.04 Start

#include "IOCP_Server.h" // IOCP 서버 헤더파일 참조

using namespace std;

//// 기본 TCP 에코 소켓 작성 후 덧붙일 예정

// 생성자 : 초기화 목록을 사용해 멤버 초기화
IOCP_Server::IOCP_Server() : iocpHandle(NULL), listenSocket(INVALID_SOCKET) {}
IOCP_Server::~IOCP_Server() { StopSvr(); }

// 서버 초기화 메서드
bool IOCP_Server::Initialize()
{
	// 1. winsock 초기화
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "ERROR : Winsock 초기화 할 수 없습니다." << endl;
		return false;
	}

	// 2. WSASocket을 사용해 비동기 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	
	if (listenSocket == INVALID_SOCKET)
	{
		cerr << "ERROR : WSASocket 생성 할 수 없습니다." << endl;
		return false;
	}

	// 3. 서버 주소 구조체 설정
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9000);

	// 4. 소켓에 주소와 포트 바인딩
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR : 소켓에 IP주소와 포트를 바인드 할 수 없습니다" << endl;
		return false;
	}

	// 5. 소켓을 접속대기 상태(리슨) 전환
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cerr << "ERROR : 리슨 상태로 전환할 수 없습니다." << endl;
		return false;
	}

	// 6. IOCP 핸들 생성
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	
	if (iocpHandle == NULL)
	{
		cerr << "ERROR : IOCP 생성할 수 없습니다." << endl;
		return false;
	}

	// 7. 워커 스레드 생성
	for (int i = 0; i < 4; ++i)
	{
		HANDLE threadHandle = CreateThread(NULL, 0, WorkerThread, this, 0, NULL);

		if (threadHandle == NULL)
		{
			cerr << "ERROR : 스레드를 생성 할 수 없습니다." << endl;
			return false;
		}
		// 스레드 핸들 저장
		workerThreads.push_back(threadHandle);
	}

	return true;
}

// 서버 시작 메서드
void IOCP_Server::StartSvr()
{
	AcceptConnections();
}

// 서버 중지 메서드
void IOCP_Server::StopSvr()
{
	// 워커 스레드 종료
	for (HANDLE thread : workerThreads)
	{
		// Post~~ 를 사용해 각 워커 스레드에 종료신호
		// IOCP 핸들에 0을 전달하여 스레드를 깨운 후 종료할 수 있게 함
		PostQueuedCompletionStatus(iocpHandle, 0, 0, NULL);

		// 각 워커 스레드가 정상 종료될 때까지 기다림
		WaitForSingleObject(thread, INFINITE);

		// 스레드가 정상 종료 후 핸들을 닫아 메모리 해제, 리소스 누수 방지
		CloseHandle(thread);
	}

	// IOCP 핸들 종료
	if (iocpHandle)
	{
		CloseHandle(iocpHandle);
		// IOCP 핸들이 더 이상 유효하지 않음
		iocpHandle = NULL;
	}

	// 리슨 소켓 종료
	if (listenSocket != INVALID_SOCKET)
	{
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}

	// Winsock 종료
	WSACleanup();
}

// 클라이언트 연결 수락 메서드
void IOCP_Server::AcceptConnections()
{
	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, NULL, NULL);

		if (clientSocket == INVALID_SOCKET)
		{
			cerr << "ERROR : 클라이언트 연결 실패되었습니다." << endl;
			continue;
		}

		// 비동기 I/O를 위한 Overlapped 구조체 할당 및 초기화
		OVERLAPPED* overlapped = new OVERLAPPED;
		ZeroMemory(overlapped, sizeof(OVERLAPPED));

		// 클라이언트 소켓을 IOCP와 연계 (IOCP CRAETE)
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)clientSocket, 0);
	
		// 클라이언트 요청 읽기
		// 클라이언트 생성 후 다시 작성
	}
}

// 클라이언트 요청 처리 메서드

// 워커 스레드 메서드
DWORD WINAPI IOCP_Server::WorkerThread(LPVOID lpParam)
{
	// 서버 인스턴스 가져오기, lpparam으로 서버 인스턴스 전달 받음
	IOCP_Server* server = (IOCP_Server*)lpParam;
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	OVERLAPPED* overlapped;

	// 무한루프 : 서버가 종료될 때까지 계속 대기
	while (true)
	{
		// 1. IOCP 큐에서 완료된 작업을 기다린다. GQCS()
		BOOL result = GetQueuedCompletionStatus(
			server->iocpHandle, // IOCP 핸들, 핸들에서 대기 중인 작업이 완료되면 반환
			&bytesTransferred, // 전송된 바이트 수
			&completionKey, // 완료키 (소켓핸들), 키 호출 시 등록된 소켓핸들이 전달됨
			&overlapped, // 비동기 작업에 사용된 OVERLAPPED 구조체
			INFINITE // 무한대기, 작업 완료될 때까지 스레드 대기상태
		);

		// 2. GQCS의 결과가 실패했을 경우 처리
		// overlapped 포인터가 NULL이 아닐 경우, 비동기 작업 리소스를 해제하여 메모리 누수 방지
		if (!result)
		{
			if (overlapped)
			{
				cerr << "ERROR : GQCS 실패" << GetLastError() << endl;
				delete overlapped;
			}
			continue; // 루프 다시 시작 다음 작업 처리 준비
		}

		// 3. completionKey 클라이언트 소켓 핸들 (iocp 큐에 등록 된 클라이언트 소켓 핸들)
		// 이 핸들을 통해 어떤 클라이언트와 작업이 완료되었는지 확인
		// 유효한 클라이언트 소켓이면, 클라이언트 요청 처리를 위해 handleclientrequest 메서드 호출
		SOCKET clientSocket = (SOCKET)completionKey;

		// 4. 정상적인 클라이언트 소켓인 경우만 처리
		if (clientSocket != INVALID_SOCKET)
		{
			// 4.1 클라이언트 요청 처리
			server->HandleClientRequest(clientSocket, overlapped);
		}

		// 5. Overlapped 구조체 메모리 해제
		// 비동기 작업 상태를 저장하는 구조체 overlapped.
		if (overlapped)
		{
			delete overlapped;
		}
	}

	return 0;
	
}

