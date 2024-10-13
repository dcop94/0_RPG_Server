#pragma once

#include "stdfx.h"
#include "DB_Manager.h"
#include "Packet.h"

class IOCP_Server
{
public:
    static IOCP_Server& Instance();

    // ������ �� �Ҹ���
    IOCP_Server();
    ~IOCP_Server();

    bool Initialiize();
    void Run();
    void Shutdown();

    void AcceptThread();
    void WorkThread();

    // ���� ����Ʈ�� ������ �� �ִ� Getter �Լ� �߰�
    SOCKET GetSessionSocket(int sessionID) const;

    void CloseSession(int sessionID);

private:
    SOCKET m_Socket;
    HANDLE m_hIOCP;
    HANDLE m_AcceptThread;
    HANDLE m_WorkThread[MAX_WORKTHREAD];

    std::vector<SOCKET> m_SessionList;
    std::deque<int> m_SessionQueue;
    mutable std::mutex m_SessionMutex;

    bool InitSocket();
    bool BindAndListen(int port);
    bool CreateAcceptThread();
    bool CreateWorkThread();
    

    // ���� �����ڿ� �Ҵ� �����ڸ� �����Ͽ� �̱��� ����
    IOCP_Server(const IOCP_Server&) = delete;
    IOCP_Server& operator=(const IOCP_Server&) = delete;
    
};

unsigned int WINAPI CallAcceptThread(LPVOID p);
unsigned int WINAPI CallWorkThread(LPVOID p);