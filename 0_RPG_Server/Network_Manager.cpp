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
	// 1. winsock �ʱ�ȭ
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "ERROR : Winsock �ʱ�ȭ �� �� �����ϴ�." << endl;
		return false;
	}

	// 2. WSASocket�� ����� �񵿱� ���� ����
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	if (listenSocket == INVALID_SOCKET)
	{
		cerr << "ERROR : WSASocket ���� �� �� �����ϴ�." << endl;
		return false;
	}

	// 3. ���� �ּ� ����ü ����
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(9000);

	// 4. ���Ͽ� �ּҿ� ��Ʈ ���ε�
	if (bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR : ���Ͽ� IP�ּҿ� ��Ʈ�� ���ε� �� �� �����ϴ�" << endl;
		return false;
	}

	// 5. ������ ���Ӵ�� ����(����) ��ȯ
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cerr << "ERROR : ���� ���·� ��ȯ�� �� �����ϴ�." << endl;
		return false;
	}

	// 6. IOCP �ڵ� ����
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

	if (iocpHandle == NULL)
	{
		cerr << "ERROR : IOCP ������ �� �����ϴ�." << endl;
		return false;
	}

	return true;
}

SOCKET NetworkManager::AcceptClient()
{
	SOCKET clientSocket = accept(listenSocket, NULL, NULL);

	if (clientSocket == INVALID_SOCKET)
	{
		cerr << "ERROR : Ŭ���̾�Ʈ ���� ���еǾ����ϴ�." << endl;
	}
	else
	{
		// Ŭ���̾�Ʈ ������ IOCP�� ���
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