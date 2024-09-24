#include <iostream>
#include "IOCP_Server.h"
#include "Database_Server.h"

using namespace std;

DWORD WINAPI WorkerThreadStart(LPVOID lpParam)
{
	WorkerThreadManager* workerThreadManager = static_cast<WorkerThreadManager*>(lpParam);
	return workerThreadManager->WorkerThread(lpParam);
}

int main()
{
	Database_Info db_info = { "tcp://127.0.0.1:3306", "user", "1234","zrpg_db" };
	int db_pool_size = 10;
	
	
	Database_Server* dbServer = new Database_Server(db_info, db_pool_size);

	IOCP_Server* iocpServer = new IOCP_Server(dbServer);

	if (!iocpServer->Initialize_iocp())
	{
		cerr << "ERROR : IOCP 서버 초기화 실패" << endl;
		return -1;
	}

	HANDLE workerThread = CreateThread(NULL, 0, WorkerThreadStart, iocpServer, 0, NULL);

	if (workerThread == NULL)
	{
		cerr << "ERROR : 워커스레드 생성 실패" << endl;
		iocpServer->StopSvr();
		return -1;
	}

	iocpServer->StartSvr();

	WaitForSingleObject(workerThread, INFINITE);

	iocpServer->StopSvr();
	delete iocpServer;
	delete dbServer;
	CloseHandle(workerThread);

	return 0;
}