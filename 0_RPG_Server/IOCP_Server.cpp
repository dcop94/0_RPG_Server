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

// 클라이언트 요청 처리 메서드

// 워커 스레드 메서드

int main()
{
	
	
	

	// 4. 클라이언트 접속 처리 및 대응
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = 0;
	char szBuffer[128] = { 0 };
	int nReceive = 0;

	// 4.1 클라이언트 연결을 받아들이고 새로운 소켓 생성(개방) 어셉트
	while ((hClient = ::accept(hSocket, (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		cout << " 새 클라이언트가 연결되었습니다. " << endl;

		// 4.2 클라이언트로부터 문자열을 수신함
		while ((nReceive = ::recv(hClient, szBuffer, sizeof(szBuffer), 0)))
		{
			// 4.3 수신한 문자열을 그대로 반향전송
			::send(hClient, szBuffer, sizeof(szBuffer), 0);
			cout << szBuffer;
			memset(szBuffer, 0, sizeof(szBuffer));
		}

		// 4.4 클라이언트가 연결을 종료함
		::shutdown(hClient, SD_BOTH);
		::closesocket(hClient);
		cout << "클라이언트 연결이 끊겼습니다" << endl;
	}

	// 5. 리슨 소켓 닫기
	::closesocket(hSocket);

	// Winsock 해제
	::WSACleanup();
	return 0;

	
}
