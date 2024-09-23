#include "Network_Manager.h"
#include <iostream>

NetworkManager::NetworkManager() : listenSocket(INVALID_SOCKET), iocpHandle(NULL) {}

NetworkManager::~NetworkManager()
{
	if (listenSocket != INVALID_SOCKET)
	{
		closesocket(listenSocket);
	}

	if (iocpHandle != NULL)
	{
		CloseHandle(iocpHandle);
	}

	WSACleanup();
}

bool NetworkManager::Initialize()
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

	return true;
}

SOCKET NetworkManager::AcceptClient()
{
	SOCKET clientSocket = accept(listenSocket, NULL, NULL);

	if (clientSocket == INVALID_SOCKET)
	{
		cerr << "ERROR : 클라이언트 연결 실패되었습니다." << endl;
	}
	else
	{
		// 클라이언트 소켓을 IOCP에 등록
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)clientSocket, 0);
	}
	return clientSocket;
}

void NetworkManager::CloseSocket(SOCKET socket)
{
	lock_guard<mutex> lock(socketMutex);
	if (socket != INVALID_SOCKET)
	{
		closesocket(socket);
	}
}

HANDLE NetworkManager::GetIOCPHandle() const
{
	return iocpHandle;
}