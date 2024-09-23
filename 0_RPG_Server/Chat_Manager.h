#pragma once
#ifndef CHAT_MANAGER_H
#define CHAT_MANAGER_H

#include <string>

using namespace std;

class ChatManager
{
public:
	void SendChatMessage(const string& Use_AccountID, const string& Use_Message);
	void BroadcastChatMessage(const string& Use_Message);
};

#endif // !CHAT_MANAGER_H
