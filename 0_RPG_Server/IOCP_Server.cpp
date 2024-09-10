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
void IOCP_Server::AcceptConnections()
{
	while (true)
	{
		SOCKET clientSocket = accept(listenSocket, NULL, NULL);

		if (clientSocket == INVALID_SOCKET)
		{
			cerr << "ERROR : Ŭ���̾�Ʈ ���� ���еǾ����ϴ�." << endl;
			continue;
		}

		// �񵿱� I/O�� ���� Overlapped ����ü �Ҵ� �� �ʱ�ȭ
		OVERLAPPED* overlapped = new OVERLAPPED;
		ZeroMemory(overlapped, sizeof(OVERLAPPED));

		// Ŭ���̾�Ʈ ������ IOCP�� ���� (IOCP CRAETE)
		CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)clientSocket, 0);
	
		// Ŭ���̾�Ʈ ��û �б�
		// Ŭ���̾�Ʈ ���� �� �ٽ� �ۼ�
	}
}

// Ŭ���̾�Ʈ ��û ó�� �޼���

// ��Ŀ ������ �޼���
DWORD WINAPI IOCP_Server::WorkerThread(LPVOID lpParam)
{
	// ���� �ν��Ͻ� ��������, lpparam���� ���� �ν��Ͻ� ���� ����
	IOCP_Server* server = (IOCP_Server*)lpParam;
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	OVERLAPPED* overlapped;

	// ���ѷ��� : ������ ����� ������ ��� ���
	while (true)
	{
		// 1. IOCP ť���� �Ϸ�� �۾��� ��ٸ���. GQCS()
		BOOL result = GetQueuedCompletionStatus(
			server->iocpHandle, // IOCP �ڵ�, �ڵ鿡�� ��� ���� �۾��� �Ϸ�Ǹ� ��ȯ
			&bytesTransferred, // ���۵� ����Ʈ ��
			&completionKey, // �Ϸ�Ű (�����ڵ�), Ű ȣ�� �� ��ϵ� �����ڵ��� ���޵�
			&overlapped, // �񵿱� �۾��� ���� OVERLAPPED ����ü
			INFINITE // ���Ѵ��, �۾� �Ϸ�� ������ ������ ������
		);

		// 2. GQCS�� ����� �������� ��� ó��
		// overlapped �����Ͱ� NULL�� �ƴ� ���, �񵿱� �۾� ���ҽ��� �����Ͽ� �޸� ���� ����
		if (!result)
		{
			if (overlapped)
			{
				cerr << "ERROR : GQCS ����" << GetLastError() << endl;
				delete overlapped;
			}
			continue; // ���� �ٽ� ���� ���� �۾� ó�� �غ�
		}

		// 3. completionKey Ŭ���̾�Ʈ ���� �ڵ� (iocp ť�� ��� �� Ŭ���̾�Ʈ ���� �ڵ�)
		// �� �ڵ��� ���� � Ŭ���̾�Ʈ�� �۾��� �Ϸ�Ǿ����� Ȯ��
		// ��ȿ�� Ŭ���̾�Ʈ �����̸�, Ŭ���̾�Ʈ ��û ó���� ���� handleclientrequest �޼��� ȣ��
		SOCKET clientSocket = (SOCKET)completionKey;

		// 4. �������� Ŭ���̾�Ʈ ������ ��츸 ó��
		if (clientSocket != INVALID_SOCKET)
		{
			// 4.1 Ŭ���̾�Ʈ ��û ó��
			server->HandleClientRequest(clientSocket, overlapped);
		}

		// 5. Overlapped ����ü �޸� ����
		// �񵿱� �۾� ���¸� �����ϴ� ����ü overlapped.
		if (overlapped)
		{
			delete overlapped;
		}
	}

	return 0;
	
}

