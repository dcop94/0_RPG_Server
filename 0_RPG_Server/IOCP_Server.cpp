// IOCP Network Server Code
// 2024.09.04 Start

#include "IOCP_Server.h" // IOCP ���� ������� ����

using namespace std;

//// �⺻ TCP ���� ���� �ۼ� �� ������ ����

// ������ : �ʱ�ȭ ����� ����� ��� �ʱ�ȭ
IOCP_Server::IOCP_Server() : iocpHandle(NULL), listenSocket(INVALID_SOCKET) {}
IOCP_Server::~IOCP_Server() { StopSvr(); }

// ���� �ʱ�ȭ �޼���
bool IOCP_Server::Initialize()
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

	// 7. ��Ŀ ������ ����
	for (int i = 0; i < 4; ++i)
	{
		HANDLE threadHandle = CreateThread(NULL, 0, WorkerThread, this, 0, NULL);

		if (threadHandle == NULL)
		{
			cerr << "ERROR : �����带 ���� �� �� �����ϴ�." << endl;
			return false;
		}
		// ������ �ڵ� ����
		workerThreads.push_back(threadHandle);
	}

	return true;
}

// ���� ���� �޼���
void IOCP_Server::StartSvr()
{
	AcceptConnections();
}

// ���� ���� �޼���
void IOCP_Server::StopSvr()
{
	// ��Ŀ ������ ����
	for (HANDLE thread : workerThreads)
	{
		// Post~~ �� ����� �� ��Ŀ �����忡 �����ȣ
		// IOCP �ڵ鿡 0�� �����Ͽ� �����带 ���� �� ������ �� �ְ� ��
		PostQueuedCompletionStatus(iocpHandle, 0, 0, NULL);

		// �� ��Ŀ �����尡 ���� ����� ������ ��ٸ�
		WaitForSingleObject(thread, INFINITE);

		// �����尡 ���� ���� �� �ڵ��� �ݾ� �޸� ����, ���ҽ� ���� ����
		CloseHandle(thread);
	}

	// IOCP �ڵ� ����
	if (iocpHandle)
	{
		CloseHandle(iocpHandle);
		// IOCP �ڵ��� �� �̻� ��ȿ���� ����
		iocpHandle = NULL;
	}

	// ���� ���� ����
	if (listenSocket != INVALID_SOCKET)
	{
		closesocket(listenSocket);
		listenSocket = INVALID_SOCKET;
	}

	// Winsock ����
	WSACleanup();
}

// Ŭ���̾�Ʈ ���� ���� �޼���

// Ŭ���̾�Ʈ ��û ó�� �޼���

// ��Ŀ ������ �޼���

int main()
{
	
	
	

	// 4. Ŭ���̾�Ʈ ���� ó�� �� ����
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = 0;
	char szBuffer[128] = { 0 };
	int nReceive = 0;

	// 4.1 Ŭ���̾�Ʈ ������ �޾Ƶ��̰� ���ο� ���� ����(����) ���Ʈ
	while ((hClient = ::accept(hSocket, (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		cout << " �� Ŭ���̾�Ʈ�� ����Ǿ����ϴ�. " << endl;

		// 4.2 Ŭ���̾�Ʈ�κ��� ���ڿ��� ������
		while ((nReceive = ::recv(hClient, szBuffer, sizeof(szBuffer), 0)))
		{
			// 4.3 ������ ���ڿ��� �״�� ��������
			::send(hClient, szBuffer, sizeof(szBuffer), 0);
			cout << szBuffer;
			memset(szBuffer, 0, sizeof(szBuffer));
		}

		// 4.4 Ŭ���̾�Ʈ�� ������ ������
		::shutdown(hClient, SD_BOTH);
		::closesocket(hClient);
		cout << "Ŭ���̾�Ʈ ������ ������ϴ�" << endl;
	}

	// 5. ���� ���� �ݱ�
	::closesocket(hSocket);

	// Winsock ����
	::WSACleanup();
	return 0;

	
}
