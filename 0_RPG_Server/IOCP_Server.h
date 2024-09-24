#pragma once
#ifndef IOCP_SERVER_H
#define IOCP_SERVER_H

#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <unordered_map>
#include <mutex>

#include "Client_Manager.h"
#include "Network_Manager.h"
#include "WorkerThread_Manager.h"
#include "Database_Server.h"
#include "Database_Login.h"
#include "Database_Character.h"

#pragma comment (lib, "ws2_32.lib")

using namespace std;

class IOCP_Server
{
public:
	// ������ �� �Ҹ���
	IOCP_Server(Database_Server* dbServerPtr); // ���� �ν��Ͻ� �ʱ�ȭ
	~IOCP_Server();

	// ���� (����, �ߴ�)
	
	bool Initialize_iocp();
	void StartSvr(); // Ŭ���̾�Ʈ ���� ��� �� ����
	void StopSvr(); // ���� �ߴ� �� ������, ���� ���� ����

private:
	NetworkManager* networkManager;
	ClientManager* clientManager;
	WorkerThreadManager* workerThreadManager;

	Database_Server* dbServer;
	Database_login* dbLogin;
	Database_character* dbCharacter;

};
#endif // !IOCP_SERVER_H
