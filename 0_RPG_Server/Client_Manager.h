#pragma once
#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <unordered_map>
#include <mutex>
#include <WinSock2.h>
#include <string>
#include "Database_Character.h"

using namespace std;

class ClientManager
{
public:
	void AddClient(SOCKET clientSocket, const struct CharacterData& ch_data);
	void RemoveClient(SOCKET clientSocket);
	void UpdateClientData(SOCKET clientSocket, const struct CharacterData& ch_data);
	CharacterData GetClientData(SOCKET clientSocket);

private:
	unordered_map<SOCKET, CharacterData> clientDataMap;
	mutex clientDataMutex;

};

#endif // !CLIENT_MANAGER_H
