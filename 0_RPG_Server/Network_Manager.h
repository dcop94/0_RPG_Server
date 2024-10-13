#pragma once
#include "stdfx.h"

class Network_Manager
{
public:
	static Network_Manager& Instance();

	Network_Manager();
	~Network_Manager();

	bool ConnectDB();
	bool ConnectToServer();
	void HandlePacket();

private:
	SOCKET m_socket;
};