#pragma once
#ifndef IOCP_SERVER_H
#define IOCP_SERVER_H

#include <iostream>
#include <WinSock2.h>
#include <winsock.h>
#include <vector>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

class IOCP_Server
{
public:
	// ������ �� �Ҹ���
	IOCP_Server(); // ���� �ν��Ͻ� �ʱ�ȭ
	~IOCP_Server();

	// ���� (�ʱ�ȭ, ����, �ߴ�)
	bool Initialize(); // ���� �����ϱ� �� ���ϰ� IOCP �ʱ�ȭ
	void StartSvr(); // Ŭ���̾�Ʈ ���� ��� �� ����
	void StopSvr(); // ���� �ߴ� �� ������, ���� ���� ����

private:
	// ��Ŀ ������ �Լ�
	static DWORD WINAPI WorkerThread(LPVOID lpParam);

	// Ŭ���̾�Ʈ ���� ����
	void AcceptConnections();

	// Ŭ���̾�Ʈ ��û ó��
	void HandleClientRequest(SOCKET clientSocket, OVERLAPPED* overlapped);

	// IOCP �ڵ�
	HANDLE iocpHandle;

	// ���� ��������
	SOCKET listenSocket;

	// ��Ŀ������ �ڵ��
	vector<HANDLE> workerThreads;
};
#endif // !IOCP_SERVER_H
