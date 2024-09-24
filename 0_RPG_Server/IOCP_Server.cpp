// IOCP Network Server Code
// 2024.09.04 Start

#include "IOCP_Server.h" // IOCP ���� ������� ����

using namespace std;

//// �⺻ TCP ���� ���� �ۼ� �� ������ ����

// ������ : �ʱ�ȭ ����� ����� ��� �ʱ�ȭ
IOCP_Server::IOCP_Server(Database_Server* dbServerPtr)
	: networkManager(nullptr), clientManager(nullptr), workerThreadManager(nullptr), 
	dbServer(dbServerPtr), dbLogin(nullptr), dbCharacter(nullptr) {}

IOCP_Server::~IOCP_Server()
{
	StopSvr();
}

// ���� �ʱ�ȭ �޼���
bool IOCP_Server::Initialize_iocp()
{
	// ��Ʈ��ũ �Ŵ��� �ʱ�ȭ
	networkManager = new NetworkManager();
	if (!networkManager->Initialize())
	{
		cerr << "ERROR : ��Ʈ��ũ �Ŵ��� �ʱ�ȭ ����" << endl;
		return false;
	}

	// Ŭ���̾�Ʈ �Ŵ��� �ʱ�ȭ
	clientManager = new ClientManager();

	// ��Ŀ ������ �Ŵ��� �ʱ�ȭ
	workerThreadManager = new WorkerThreadManager(clientManager, networkManager, dbLogin, dbCharacter);


	// �����ͺ��̽� ���� �ʱ�ȭ
	dbLogin = new Database_login(dbServer);
	dbCharacter = new Database_character(dbServer);

	if (!dbLogin || !dbCharacter)
	{
		cerr << "ERROR : �����ͺ��̽� �ʱ�ȭ ����" << endl;
		return false;
	}

	return true;
}

// ���� ���� �޼���
void IOCP_Server::StartSvr()
{
	// Ŭ���̾�Ʈ ���� ó��
	while (true)
	{
		SOCKET clientSocket = networkManager->AcceptClient();
		if (clientSocket != INVALID_SOCKET)
		{
			// ���ο� Ŭ���̾�Ʈ�� ���� �ʱ� ������ ó�� (������ �� �ε�)
			string Use_AccountID; // Ŭ���̾�Ʈ�κ��� ���� ����� ���� ID
			CharacterData ch_data;

			if (!dbCharacter->getCharacterInfo(Use_AccountID, ch_data))
			{
				cerr << "ERROR : Ŭ���̾�Ʈ ĳ���� ���� �ҷ����� ����" << endl;
			}
			else
			{
				// Ŭ���̾�Ʈ �Ŵ����� �߰�
				clientManager->AddClient(clientSocket, ch_data);
			}

		}
	}
}

// ���� ���� �޼���
void IOCP_Server::StopSvr()
{
	if (networkManager)
	{
		delete networkManager;
		networkManager = nullptr;
	}

	if (clientManager)
	{
		delete clientManager;
		clientManager = nullptr;
	}

	if (workerThreadManager)
	{
		delete workerThreadManager;
		workerThreadManager = nullptr;
	}

	if (dbServer)
	{
		delete dbServer;
		dbServer = nullptr;
	}

	if (dbLogin)
	{
		delete dbLogin;
		dbLogin = nullptr;
	}

	if (dbCharacter)
	{
		delete dbCharacter;
		dbCharacter = nullptr;
	}

	WSACleanup();
}

