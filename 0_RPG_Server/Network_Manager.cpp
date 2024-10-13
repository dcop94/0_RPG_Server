#include "Network_Manager.h"
#include "DB_Manager.h"

Network_Manager& Network_Manager::Instance() 
{
	static Network_Manager instance;
	return instance;
}

Network_Manager::Network_Manager()
{
	m_socket = INVALID_SOCKET;
}

Network_Manager::~Network_Manager()
{
	closesocket(m_socket);
	WSACleanup();
}

bool Network_Manager::ConnectDB()
{
	return _DB_Manager.CheckDBConnection();
}

bool Network_Manager::ConnectToServer()
{
	return true;
}

void Network_Manager::HandlePacket()
{

}