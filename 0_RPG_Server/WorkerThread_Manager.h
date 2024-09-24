#pragma once
#ifndef WORKERTHREAD_MANAGER_H
#define WORKERTHREAD_MANAGER_H

#include <WinSock2.h>
#include "Client_Manager.h"
#include "Network_Manager.h"
#include "Database_Login.h"
#include "Database_Character.h"

class WorkerThreadManager
{
public:
	WorkerThreadManager(ClientManager* clientMG, NetworkManager* netMG, Database_login* dbLogin, Database_character* dbCharacter);
	static DWORD WINAPI WorkerThread(LPVOID lpParam);

private:
	ClientManager* clientManager;
	NetworkManager* networkManager;

	Database_login* dbLogin;
	Database_character* dbCharacter;

	void HandleClientRequest(SOCKET clientSocket, OVERLAPPED* overlapped);
	void HandleLoginRequest(SOCKET clientSocket, const char* data, int dataSize);
	void HandleCharacterInfoRequest(SOCKET clientSocket, const char* data, int dataSize);
	
	void SendLoginResponse(SOCKET clientSocket, bool success, const string& message);
	void SendCharacterInfoResponse(SOCKET clientSocket, bool success, const string& message, const CharacterData& characterData);

};

#endif // !WORKERTHREAD_MANAGER_H
