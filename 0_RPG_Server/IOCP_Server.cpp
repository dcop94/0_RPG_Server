// IOCP Network Server Code
// 2024.09.04 Start

#include "IOCP_Server.h" // IOCP ���� ������� ����

using namespace std;

//// �⺻ TCP ���� ���� �ۼ� �� ������ ����

// ������ : �ʱ�ȭ ����� ����� ��� �ʱ�ȭ
IOCP_Server::IOCP_Server() 
	: networkManager(nullptr), clientManager(nullptr), workerThreadManager(nullptr), 
	dbServer(nullptr), dbLogin(nullptr), dbCharacter(nullptr) {}
IOCP_Server::~IOCP_Server() { StopSvr(); }

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
	workerThreadManager = new WorkerThreadManager(clientManager, networkManager);


	// �����ͺ��̽� ���� �ʱ�ȭ
	Database_Info dbInfo = { "localhost", "root", "password", "game_schema" };
	dbServer = new Database_Server(dbInfo);
	dbLogin = new Database_login(dbServer);
	dbCharacter = new Database_character(dbServer);

	if (!dbServer || !dbLogin || !dbCharacter)
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
			CharacterData ch_data = dbCharacter->getCharacterInfo(Use_AccountID);

			// Ŭ���̾�Ʈ �Ŵ����� �߰�
			clientManager->AddClient(clientSocket, ch_data);
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

