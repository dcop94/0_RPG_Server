// IOCP Network Server Code
// 2024.09.04 Start

#include <iostream>
#include <WinSock2.h>
#include <string>
#include <vector>

#pragma comment(lib, "ws2_32.lib") // ws2_32.lib ���̺귯�� ��ũ 

using namespace std;

//// �⺻ TCP ���� ���� �ۼ� �� ������ ����

int main()
{
	// winsock �ʱ�ȭ
	WSADATA wsa = { 0 };
	if (::WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		cerr << "ERROR : Winsock �ʱ�ȭ �� �� �����ϴ�." << endl;
		return 1;
	}
	// 1. ���ϻ���
	SOCKET hSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (hSocket == INVALID_SOCKET)
	{
		cerr << "ERROR : ���� ��� ������ ���� �� �� �����ϴ�." << endl;
		return 11;
	}

	// 2. ��Ʈ ���ε�
	SOCKADDR_IN svraddr = { 0 };
	svraddr.sin_family = AF_INET;
	svraddr.sin_port = htons(25000);
	svraddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (::bind(hSocket, (SOCKADDR*)&svraddr, sizeof(svraddr)) == SOCKET_ERROR)
	{
		cerr << "ERROR : ���Ͽ� IP�ּҿ� ��Ʈ�� ���ε� �� �� �����ϴ�." << endl;
		return 11;
	}

	// 3. ���Ӵ�� ���� (����) ��ȯ
	if (::listen(hSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		cerr << "ERROR : ���� ���·� ��ȯ�� �� �����ϴ�." << endl;
		return 11;
	}

	// 4. Ŭ���̾�Ʈ ���� ó�� �� ����
	SOCKADDR_IN clientaddr = { 0 };
	int nAddrLen = sizeof(clientaddr);
	SOCKET hClient = 0;
	char szBuffer[128] = { 0 };
	int nReceive = 0;

	// 4.1 Ŭ���̾�Ʈ ������ �޾Ƶ��̰� ���ο� ���� ����(����) ���Ʈ
	while ((hClient = ::accept(hSocket, (SOCKADDR*)&clientaddr, &nAddrLen)) != INVALID_SOCKET)
	{
		cout << " �� Ŭ���̾�Ʈ�� ����Ǿ����ϴ�. " << endl;

		// 4.2 Ŭ���̾�Ʈ�κ��� ���ڿ��� ������
		while ((nReceive = ::recv(hClient, szBuffer, sizeof(szBuffer), 0)))
		{
			// 4.3 ������ ���ڿ��� �״�� ��������
			::send(hClient, szBuffer, sizeof(szBuffer), 0);
			cout << szBuffer;
			memset(szBuffer, 0, sizeof(szBuffer));
		}

		// 4.4 Ŭ���̾�Ʈ�� ������ ������
		::shutdown(hClient, SD_BOTH);
		::closesocket(hClient);
		cout << "Ŭ���̾�Ʈ ������ ������ϴ�" << endl;
	}

	// 5. ���� ���� �ݱ�
	::closesocket(hSocket);

	// Winsock ����
	::WSACleanup();
	return 0;

	
}
