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
	// 생성자 및 소멸자
	IOCP_Server(); // 서버 인스턴스 초기화
	~IOCP_Server();

	// 서버 (초기화, 시작, 중단)
	bool Initialize(); // 서버 시작하기 전 소켓과 IOCP 초기화
	void StartSvr(); // 클라이언트 연결 대기 및 수락
	void StopSvr(); // 서버 중단 및 스레드, 소켓 등을 해제

private:
	// 워커 스레드 함수
	static DWORD WINAPI WorkerThread(LPVOID lpParam);

	// 클라이언트 연결 수락
	void AcceptConnections();

	// 클라이언트 요청 처리
	void HandleClientRequest(SOCKET clientSocket, OVERLAPPED* overlapped);

	// IOCP 핸들
	HANDLE iocpHandle;

	// 서버 리슨소켓
	SOCKET listenSocket;

	// 워커스레드 핸들들
	vector<HANDLE> workerThreads;
};
#endif // !IOCP_SERVER_H
