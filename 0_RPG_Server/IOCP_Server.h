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
	// 생성자 및 소멸자
	IOCP_Server(Database_Server* dbServerPtr); // 서버 인스턴스 초기화
	~IOCP_Server();

	// 서버 (시작, 중단)
	
	bool Initialize_iocp();
	void StartSvr(); // 클라이언트 연결 대기 및 수락
	void StopSvr(); // 서버 중단 및 스레드, 소켓 등을 해제

private:
	NetworkManager* networkManager;
	ClientManager* clientManager;
	WorkerThreadManager* workerThreadManager;

	Database_Server* dbServer;
	Database_login* dbLogin;
	Database_character* dbCharacter;

};
#endif // !IOCP_SERVER_H
