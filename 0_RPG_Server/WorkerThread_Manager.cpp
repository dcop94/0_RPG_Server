#include "WorkerThread_Manager.h"
#include "Database_Login.h"
#include "Database_Character.h"
#include <iostream>


WorkerThreadManager::WorkerThreadManager(ClientManager* clientMG, NetworkManager* netMG, Database_login* dbLogin, Database_character* dbCharacter)
	: clientManager(clientMG), networkManager(netMG), dbLogin(dbLogin), dbCharacter(dbCharacter) {}


DWORD WINAPI WorkerThreadManager::WorkerThread(LPVOID lpParam)
{
	// 서버 인스턴스 가져오기, lpparam으로 서버 인스턴스 전달 받음
	WorkerThreadManager* workerMG = (WorkerThreadManager*)lpParam;
	DWORD bytesTransferred;
	ULONG_PTR completionKey;
	OVERLAPPED* overlapped;

	HANDLE iocpHandle = workerMG->networkManager->GetIOCPHandle();

	// 무한루프 : 서버가 종료될 때까지 계속 대기
	while (true)
	{
		// 1. IOCP 큐에서 완료된 작업을 기다린다. GQCS()
		BOOL result = GetQueuedCompletionStatus(
			iocpHandle, // IOCP 핸들, 핸들에서 대기 중인 작업이 완료되면 반환
			&bytesTransferred, // 전송된 바이트 수
			&completionKey, // 완료키 (소켓핸들), 키 호출 시 등록된 소켓핸들이 전달됨
			&overlapped, // 비동기 작업에 사용된 OVERLAPPED 구조체
			INFINITE // 무한대기, 작업 완료될 때까지 스레드 대기상태
		);

		// 2. GQCS의 결과가 실패했을 경우 처리
		// overlapped 포인터가 NULL이 아닐 경우, 비동기 작업 리소스를 해제하여 메모리 누수 방지
		if (!result)
		{
			cerr << "ERROR : GQCS 실패" << GetLastError() << endl;
			continue;
		}

		// 3. completionKey 클라이언트 소켓 핸들 (iocp 큐에 등록 된 클라이언트 소켓 핸들)
		// 이 핸들을 통해 어떤 클라이언트와 작업이 완료되었는지 확인
		// 유효한 클라이언트 소켓이면, 클라이언트 요청 처리를 위해 handleclientrequest 메서드 호출
		SOCKET clientSocket = (SOCKET)completionKey;

		// 4. 정상적인 클라이언트 소켓인 경우만 처리
		if (clientSocket != INVALID_SOCKET)
		{
			// 4.1 클라이언트 요청 처리
			workerMG->HandleClientRequest(clientSocket, overlapped);
		}

		// 5. Overlapped 구조체 메모리 해제
		// 비동기 작업 상태를 저장하는 구조체 overlapped.
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
		
		// 요청 구분
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

	// 데이터 파싱 (이메일, 비밀번호 추출)
	size_t delimiterPos = receivedData.find(',');

	if (delimiterPos == string::npos)
	{
		SendLoginResponse(clientSocket, false, "잘못된 요청 형식");
		return;
	}

	string email = receivedData.substr(0, delimiterPos);
	string password = receivedData.substr(delimiterPos + 1);

	// Database_login 객체 사용하여 로그인 검증
	if (dbLogin->verifyLogin(email, password))
	{
		// 로그인 성공
		SendLoginResponse(clientSocket, true, "로그인 성공");
	}
	else
	{
		// 로그인 실패
		SendLoginResponse(clientSocket, false, "로그인 실패");
	}
}

void WorkerThreadManager::HandleCharacterInfoRequest(SOCKET clientSocket, const char* data, int dataSize)
{
	string accountCode(data, dataSize);

	// Database_Character에서 캐릭터 정보 가져오기
	CharacterData characterData;

	if (dbCharacter->getCharacterInfo(accountCode, characterData))
	{
		// 캐릭터 정보를 가져옴
		SendCharacterInfoResponse(clientSocket, true, "캐릭터 정보 조회 성공", characterData);
	}
	else
	{
		// 캐릭터 정보 가져오기 실패
		SendCharacterInfoResponse(clientSocket, false, "캐릭터 정보 조회 실패", characterData);
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