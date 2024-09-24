// IOCP Network Server Code
// 2024.09.04 Start

#include "IOCP_Server.h" // IOCP 서버 헤더파일 참조

using namespace std;

//// 기본 TCP 에코 소켓 작성 후 덧붙일 예정

// 생성자 : 초기화 목록을 사용해 멤버 초기화
IOCP_Server::IOCP_Server(Database_Server* dbServerPtr)
	: networkManager(nullptr), clientManager(nullptr), workerThreadManager(nullptr), 
	dbServer(dbServerPtr), dbLogin(nullptr), dbCharacter(nullptr) {}

IOCP_Server::~IOCP_Server()
{
	StopSvr();
}

// 서버 초기화 메서드
bool IOCP_Server::Initialize_iocp()
{
	// 네트워크 매니저 초기화
	networkManager = new NetworkManager();
	if (!networkManager->Initialize())
	{
		cerr << "ERROR : 네트워크 매니저 초기화 실패" << endl;
		return false;
	}

	// 클라이언트 매니저 초기화
	clientManager = new ClientManager();

	// 워커 스레드 매니저 초기화
	workerThreadManager = new WorkerThreadManager(clientManager, networkManager, dbLogin, dbCharacter);


	// 데이터베이스 서버 초기화
	dbLogin = new Database_login(dbServer);
	dbCharacter = new Database_character(dbServer);

	if (!dbLogin || !dbCharacter)
	{
		cerr << "ERROR : 데이터베이스 초기화 실패" << endl;
		return false;
	}

	return true;
}

// 서버 시작 메서드
void IOCP_Server::StartSvr()
{
	// 클라이언트 연결 처리
	while (true)
	{
		SOCKET clientSocket = networkManager->AcceptClient();
		if (clientSocket != INVALID_SOCKET)
		{
			// 새로운 클라이언트에 대해 초기 데이터 처리 (재접속 시 로드)
			string Use_AccountID; // 클라이언트로부터 받은 사용자 고유 ID
			CharacterData ch_data;

			if (!dbCharacter->getCharacterInfo(Use_AccountID, ch_data))
			{
				cerr << "ERROR : 클라이언트 캐릭터 정보 불러오기 실패" << endl;
			}
			else
			{
				// 클라이언트 매니저에 추가
				clientManager->AddClient(clientSocket, ch_data);
			}

		}
	}
}

// 서버 중지 메서드
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

