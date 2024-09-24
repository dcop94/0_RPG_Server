#include "WorkerThread_Manager.h"
#include "Database_Login.h"
#include "Database_Character.h"
#include <iostream>


WorkerThreadManager::WorkerThreadManager(ClientManager* clientMG, NetworkManager* netMG, Database_login* dbLogin, Database_character* dbCharacter)
	: clientManager(clientMG), networkManager(netMG), dbLogin(dbLogin), dbCharacter(dbCharacter) {}


DWORD WINAPI WorkerThreadManager::WorkerThread(LPVOID lpParam)
{
	// ���� �ν��Ͻ� ��������, lpparam���� ���� �ν��Ͻ� ���� ����
	WorkerThreadManager* workerMG = (WorkerThreadManager*)lpParam;
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	OVERLAPPED* overlapped;

	HANDLE iocpHandle = workerMG->networkManager->GetIOCPHandle();

	// ���ѷ��� : ������ ����� ������ ��� ���
	while (true)
	{
		// 1. IOCP ť���� �Ϸ�� �۾��� ��ٸ���. GQCS()
		BOOL result = GetQueuedCompletionStatus(
			iocpHandle, // IOCP �ڵ�, �ڵ鿡�� ��� ���� �۾��� �Ϸ�Ǹ� ��ȯ
			&bytesTransferred, // ���۵� ����Ʈ ��
			&completionKey, // �Ϸ�Ű (�����ڵ�), Ű ȣ�� �� ��ϵ� �����ڵ��� ���޵�
			&overlapped, // �񵿱� �۾��� ���� OVERLAPPED ����ü
			INFINITE // ���Ѵ��, �۾� �Ϸ�� ������ ������ ������
		);

		// 2. GQCS�� ����� �������� ��� ó��
		// overlapped �����Ͱ� NULL�� �ƴ� ���, �񵿱� �۾� ���ҽ��� �����Ͽ� �޸� ���� ����
		if (!result)
		{
			cerr << "ERROR : GQCS ����" << GetLastError() << endl;
			continue;
		}

		// 3. completionKey Ŭ���̾�Ʈ ���� �ڵ� (iocp ť�� ��� �� Ŭ���̾�Ʈ ���� �ڵ�)
		// �� �ڵ��� ���� � Ŭ���̾�Ʈ�� �۾��� �Ϸ�Ǿ����� Ȯ��
		// ��ȿ�� Ŭ���̾�Ʈ �����̸�, Ŭ���̾�Ʈ ��û ó���� ���� handleclientrequest �޼��� ȣ��
		SOCKET clientSocket = (SOCKET)completionKey;

		// 4. �������� Ŭ���̾�Ʈ ������ ��츸 ó��
		if (clientSocket != INVALID_SOCKET)
		{
			// 4.1 Ŭ���̾�Ʈ ��û ó��
			workerMG->HandleClientRequest(clientSocket, overlapped);
		}

		// 5. Overlapped ����ü �޸� ����
		// �񵿱� �۾� ���¸� �����ϴ� ����ü overlapped.
		if (overlapped)
		{
			delete overlapped;
		}
	}

	return 0;
}

void WorkerThreadManager::HandleClientRequest(SOCKET clientSocket, OVERLAPPED* overlapped)
{
	char recvBuffer[1024];
	int recvSize = recv(clientSocket, recvBuffer, sizeof(recvBuffer), 0);

	if (recvSize > 0)
	{
		string receivedData(recvBuffer, recvSize);
		
		// ��û ����
		if (receivedData.find("LOGIN") == 0)
		{
			HandleLoginRequest(clientSocket, recvBuffer + 6, recvSize - 6);
		}
		else if (receivedData.find("CHAR_INFO") == 0)
		{
			HandleCharacterInfoRequest(clientSocket, recvBuffer + 9, recvSize - 9);
		}

	}
		
}

void WorkerThreadManager::HandleLoginRequest(SOCKET clientSocket, const char* data, int dataSize)
{
	string receivedData(data, dataSize);

	// ������ �Ľ� (�̸���, ��й�ȣ ����)
	size_t delimiterPos = receivedData.find(',');

	if (delimiterPos == string::npos)
	{
		SendLoginResponse(clientSocket, false, "�߸��� ��û ����");
		return;
	}

	string email = receivedData.substr(0, delimiterPos);
	string password = receivedData.substr(delimiterPos + 1);

	// Database_login ��ü ����Ͽ� �α��� ����
	if (dbLogin->verifyLogin(email, password))
	{
		// �α��� ����
		SendLoginResponse(clientSocket, true, "�α��� ����");
	}
	else
	{
		// �α��� ����
		SendLoginResponse(clientSocket, false, "�α��� ����");
	}
}

void WorkerThreadManager::HandleCharacterInfoRequest(SOCKET clientSocket, const char* data, int dataSize)
{
	string accountCode(data, dataSize);

	// Database_Character���� ĳ���� ���� ��������
	CharacterData characterData;

	if (dbCharacter->getCharacterInfo(accountCode, characterData))
	{
		// ĳ���� ������ ������
		SendCharacterInfoResponse(clientSocket, true, "ĳ���� ���� ��ȸ ����", characterData);
	}
	else
	{
		// ĳ���� ���� �������� ����
		SendCharacterInfoResponse(clientSocket, false, "ĳ���� ���� ��ȸ ����", characterData);
	}
}

void WorkerThreadManager::SendLoginResponse(SOCKET clientSocket, bool success, const string& message)
{
	string response = success ? "SUCCES : " + message : "FAIL :" + message;
	
	send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
}

void WorkerThreadManager::SendCharacterInfoResponse(SOCKET clientSocket, bool success, const string& message, const CharacterData& characterData)
{
	string response;

	if (success)
	{
		response = "SUCCESS :" + message + "\n";
		response += "CharacterClass :" + characterData.Use_Char_Class + "\n";
		response += "UseName :" + characterData.Use_Char_Name + "\n";
		response += "MapLocation :" + to_string(characterData.positionX) + "," + to_string(characterData.positionY) + "\n";
		response += "LEVEL :" + to_string(characterData.Use_level) + "\n";
		response += "EXP :" + to_string(characterData.Use_exp) + "\n";
		response += "HP :" + to_string(characterData.Use_hp) + "\n";
		response += "MP :" + to_string(characterData.Use_mp) + "\n";

	}
	else
	{
		response = "FAIL : " + message;
	}

	send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
}