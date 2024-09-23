#pragma once
#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <WinSock2.h>
#include <vector>
#include <mutex>

using namespace std;

class NetworkManager
{
public:
	NetworkManager();
	~NetworkManager();

	bool Initialize(); // ���� �����ϱ� �� ���ϰ� IOCP �ʱ�ȭ
	SOCKET AcceptClient();
	void CloseSocket(SOCKET socket);
	HANDLE GetIOCPHandle() const; 

private:
	SOCKET listenSocket;
	HANDLE iocpHandle;
	mutex socketMutex;
};

#endif // !NETWORK_MANAGER_H
