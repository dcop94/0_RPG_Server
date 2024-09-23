#pragma once
#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H

#include <unordered_map>
#include <mutex>
#include <WinSock2.h>
#include <string>

using namespace std;

struct CharacterData
{
	string Use_AccountID;
	string Use_Char_Class;
	string Use_Char_Name;
	float positionX;
	float positionY;
	int Use_level;
	double Use_exp;
	int Use_hp;
	int Use_mp;
};

class ClientManager
{
public:
	void AddClient(SOCKET clientSocket, const CharacterData& ch_data);
	void RemoveClient(SOCKET clientSocket);
	void UpdateClientData(SOCKET clientSocket, const CharacterData& ch_data);
	CharacterData GetClientData(SOCKET clientSocket);

private:
	unordered_map<SOCKET, CharacterData> clientDataMap;
	mutex clientDataMutex;

};

#endif // !CLIENT_MANAGER_H
