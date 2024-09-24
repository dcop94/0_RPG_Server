#include "Client_Manager.h"

void ClientManager::AddClient(SOCKET clientSocket, const struct CharacterData& ch_data)
{
	lock_guard<mutex> lock(clientDataMutex);
	clientDataMap[clientSocket] = ch_data;
}

void ClientManager::RemoveClient(SOCKET clientSocket)
{
	lock_guard<mutex> lock(clientDataMutex);
	clientDataMap.erase(clientSocket);
}

void ClientManager::UpdateClientData(SOCKET clientSocket, const struct CharacterData& ch_data)
{
	lock_guard<mutex> lock(clientDataMutex);
	clientDataMap[clientSocket] = ch_data;
}

CharacterData ClientManager::GetClientData(SOCKET clientSocket)
{
	lock_guard<mutex> lock(clientDataMutex);
	return clientDataMap[clientSocket];
}