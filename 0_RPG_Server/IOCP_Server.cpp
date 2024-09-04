// IOCP Network Server Code
// 2024.09.04 Start

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib") // ws2_32.lib 라이브러리 링크 

using namespace std;

//// 기본 TCP 에코 소켓 작성 후 덧붙일 예정

int main()
{
	// winsock 초기화
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "ERROR : Winsock 초기화 할 수 없습니다." << endl;
		return 1;
	}
	// 1. 소켓생성
	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		cerr << "ERROR : 접속 대기 소켓을 생성 할 수 없습니다." << endl;
		return 11;
	}

	// 2. 포트 바인딩
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR : 소켓에 IP주소와 포트를 바인드 할 수 없습니다." << endl;
		return 11;
	}

	// 3. 접속대기 상태 (리슨) 전환
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cerr << "ERROR : 리슨 상태로 전환할 수 없습니다." << endl;
		return 11;
	}

	// 4. 클라이언트 접속 처리 및 대응
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = 0;
	char szBuffer[128] = { 0 };
	int nReceive = 0;

	// 4.1 클라이언트 연결을 받아들이고 새로운 소켓 생성(개방) 어셉트
	while ((hClient = ::accept(hSocket, (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		cout << " 새 클라이언트가 연결되었습니다. " << endl;

		// 4.2 클라이언트로부터 문자열을 수신함
		while ((nReceive = ::recv(hClient, szBuffer, sizeof(szBuffer), 0)))
		{
			// 4.3 수신한 문자열을 그대로 반향전송
			::send(hClient, szBuffer, sizeof(szBuffer), 0);
			cout << szBuffer;
			memset(szBuffer, 0, sizeof(szBuffer));
		}

		// 4.4 클라이언트가 연결을 종료함
		::shutdown(hClient, SD_BOTH);
		::closesocket(hClient);
		cout << "클라이언트 연결이 끊겼습니다" << endl;
	}

	// 5. 리슨 소켓 닫기
	::closesocket(hSocket);

	// Winsock 해제
	::WSACleanup();
	return 0;

	
}
