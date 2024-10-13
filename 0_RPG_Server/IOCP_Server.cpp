#include "IOCP_Server.h"

IOCP_Server& IOCP_Server::Instance()
{
    static IOCP_Server instance;
    return instance;
}

IOCP_Server::IOCP_Server() : m_Socket(INVALID_SOCKET), m_hIOCP(NULL)
{
    InitSocket();
    BindAndListen(PORTNUM);

    for (size_t i = 0; i < MAX_CLIENT_COUNT; ++i)
    {
        m_SessionList.push_back(INVALID_SOCKET);
        m_SessionQueue.push_back(static_cast<int>(i));
    }

    // m_WorkThread 배열 초기화
    for (int i = 0; i < MAX_WORKTHREAD; ++i)
    {
        m_WorkThread[i] = NULL;
    }
}

IOCP_Server::~IOCP_Server()
{
    WSACleanup();

    for (size_t i = 0; i < m_SessionList.size(); ++i) {
        if (m_SessionList[i] != INVALID_SOCKET) {
            shutdown(m_SessionList[i], SD_BOTH);
            closesocket(m_SessionList[i]);
        }
    }

    for (int i = 0; i < MAX_WORKTHREAD; ++i) {
        PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
    }
    for (int i = 0; i < MAX_WORKTHREAD; ++i) {
        if (m_WorkThread[i] != NULL)
        {
            WaitForSingleObject(m_WorkThread[i], INFINITE);
            CloseHandle(m_WorkThread[i]);
            m_WorkThread[i] = NULL;
        }
    }
}

// 세션 소켓에 접근할 수 있는 Getter 함수 구현
SOCKET IOCP_Server::GetSessionSocket(int sessionID) const
{
    std::lock_guard<std::mutex> lock(m_SessionMutex);  // 뮤텍스 잠금

    if (sessionID < 0 || sessionID >= (int)m_SessionList.size())
    {
        std::cerr << "[ERROR] GetSessionSocket: Invalid session ID " << sessionID << std::endl;
        return INVALID_SOCKET;
    }

    SOCKET sessionSocket = m_SessionList[sessionID];
    std::cout << "[DEBUG] GetSessionSocket: Session socket for session ID " << sessionID << " is " << sessionSocket << std::endl;

    if (sessionSocket == INVALID_SOCKET)
    {
        std::cerr << "[ERROR] GetSessionSocket: Session socket is INVALID_SOCKET for session ID " << sessionID << std::endl;
    }
    return sessionSocket;
}

bool IOCP_Server::InitSocket()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cout << "[ERROR] WinSock initialization failed." << std::endl;
        return false;
    }

    m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
    if (m_Socket == INVALID_SOCKET)
    {
        std::cout << "[ERROR] Socket creation failed." << std::endl;
        return false;
    }

    int option = TRUE;
    setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&option, sizeof(option));

    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    return m_hIOCP != NULL;
}

bool IOCP_Server::BindAndListen(int Port)
{
    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORTNUM);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = bind(m_Socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (ret != 0)
    {
        std::cout << "[ERROR] Bind failed." << std::endl;
        return false;
    }

    ret = listen(m_Socket, SOMAXCONN);
    if (ret != 0)
    {
        std::cout << "[ERROR] Listen failed." << std::endl;
        return false;
    }

    return true;
}

bool IOCP_Server::Initialiize()
{
    bool ret = CreateAcceptThread();
    if (!ret) return false;

    ret = CreateWorkThread();
    return ret;
}

void IOCP_Server::Run()
{
    // 메인 스레드는 대기 상태로 둡니다.
    WaitForSingleObject(m_AcceptThread, INFINITE);
}

bool IOCP_Server::CreateAcceptThread()
{
    unsigned int ThreadId = 0;
    m_AcceptThread = (HANDLE)_beginthreadex(NULL, 0, &CallAcceptThread, this, CREATE_SUSPENDED, &ThreadId);
    if (m_AcceptThread == NULL)
    {
        std::cout << "[ERROR] Accept thread creation failed." << std::endl;
        return false;
    }
    ResumeThread(m_AcceptThread);
    return true;
}

bool IOCP_Server::CreateWorkThread()
{
    unsigned int ThreadID = 0;
    for (size_t i = 0; i < MAX_WORKTHREAD; ++i)
    {
        m_WorkThread[i] = (HANDLE)_beginthreadex(NULL, 0, &CallWorkThread, this, CREATE_SUSPENDED, &ThreadID);
        if (m_WorkThread[i] == NULL)
        {
            std::cout << "[ERROR] Work thread creation failed." << std::endl;
            return false;
        }
        ResumeThread(m_WorkThread[i]);
    }
    return true;
}

void IOCP_Server::AcceptThread()
{
    SOCKADDR_IN clientAddr;
    int addrLen = sizeof(SOCKADDR_IN);

    std::cout << "[INFO] Accept thread started." << std::endl;

    while (true)
    {
        int sessionID = -1;

        // 세션 큐 접근 보호
        {
            std::lock_guard<std::mutex> lock(m_SessionMutex);
            if (m_SessionQueue.empty())
            {
                std::cerr << "[ERROR] Session queue is empty. Cannot accept new clients." << std::endl;
                Sleep(100);  // 잠시 대기 후 다시 시도
                continue;
            }

            sessionID = m_SessionQueue.front();
            m_SessionQueue.pop_front();
        }

        SOCKET clientSocket = accept(m_Socket, (SOCKADDR*)&clientAddr, &addrLen);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "[ERROR] accept failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }

        // 세션 리스트에 소켓 설정 보호
        {
            std::lock_guard<std::mutex> lock(m_SessionMutex);
            m_SessionList[sessionID] = clientSocket;
            std::cout << "[DEBUG] Session " << sessionID << " socket set to " << clientSocket << " in AcceptThread" << std::endl;  // 추가된 로그
        }

        // IOCP에 소켓 등록
        HANDLE hIOCP = CreateIoCompletionPort((HANDLE)clientSocket, m_hIOCP, (ULONG_PTR)(sessionID), 0);
        if (hIOCP == NULL || m_hIOCP != hIOCP)
        {
            std::cerr << "[ERROR] IOCP connection error." << std::endl;
            closesocket(clientSocket);
            {
                std::lock_guard<std::mutex> lock(m_SessionMutex);
                m_SessionList[sessionID] = INVALID_SOCKET;
                m_SessionQueue.push_back(sessionID);
            }
            std::cout << "[DEBUG] Session " << sessionID << " socket set to INVALID_SOCKET due to IOCP error" << std::endl;
            continue;
        }

        // 첫 번째 WSARecv 호출
        Buffer* pBuffer = new Buffer();
        DWORD flags = 0;
        ZeroMemory(&pBuffer->overlapped, sizeof(OVERLAPPED));
        int recvResult = WSARecv(clientSocket, &pBuffer->wsabuf, 1, NULL, &flags, &pBuffer->overlapped, NULL);
        if (recvResult == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSA_IO_PENDING)
            {
                std::cerr << "[ERROR] Initial WSARecv failed with error: " << error << std::endl;
                closesocket(clientSocket);
                {
                    std::lock_guard<std::mutex> lock(m_SessionMutex);
                    m_SessionList[sessionID] = INVALID_SOCKET;
                    m_SessionQueue.push_back(sessionID);
                }
                std::cout << "[DEBUG] Session " << sessionID << " socket set to INVALID_SOCKET due to initial WSARecv error" << std::endl;
                delete pBuffer;
                continue;
            }
        }
    }
}

void IOCP_Server::WorkThread()
{
    DWORD ioSize;
    ULONG_PTR completionKey;
    Buffer* pBuffer = NULL;

    while (true)
    {
        bool result = GetQueuedCompletionStatus(m_hIOCP, &ioSize, &completionKey, (LPOVERLAPPED*)&pBuffer, INFINITE);
        int sessionID = static_cast<int>(completionKey);

        std::cout << "[DEBUG] GetQueuedCompletionStatus returned. sessionID: " << sessionID
            << ", ioSize: " << ioSize
            << ", opType: " << (pBuffer ? pBuffer->opType : -1) << std::endl;

        if (pBuffer == NULL)
        {
            // 종료 신호 처리
            break;
        }

        if (!result)
        {
            int error = GetLastError();
            if (error == ERROR_NETNAME_DELETED || error == ERROR_CONNECTION_ABORTED)
            {
                std::cout << "[INFO] Client disconnected: " << sessionID << std::endl;
            }
            else
            {
                std::cerr << "[ERROR] GetQueuedCompletionStatus failed with error: " << error << std::endl;
            }
            CloseSession(sessionID);
            delete pBuffer;
            continue;
        }

        if (pBuffer->opType == OP_READ)
        {
            if (ioSize == 0)
            {
                // 클라이언트가 연결을 끊은 경우
                std::cout << "[INFO] Connection closed by client: " << sessionID << std::endl;
                CloseSession(sessionID);
                delete pBuffer;
                continue;
            }

            // 읽기 작업 처리
            std::cout << "Data received from session " << sessionID << std::endl;
            std::string receivedData(pBuffer->buffer, ioSize);
            std::cout << "[DEBUG] Received data: " << receivedData << std::endl;

            // 패킷 처리 로직
            std::stringstream inputStream;
            inputStream.write(pBuffer->buffer, ioSize);

            _Packek.CheckPacket(inputStream, sessionID);

            // **여기서 새로운 WSARecv를 호출함**
            ZeroMemory(&pBuffer->overlapped, sizeof(OVERLAPPED));
            pBuffer->opType = OP_READ;
            DWORD flags = 0;

            SOCKET clientSocket;
            {
                std::lock_guard<std::mutex> lock(m_SessionMutex);
                clientSocket = m_SessionList[sessionID];
            }

            int recvResult = WSARecv(clientSocket, &pBuffer->wsabuf, 1, NULL, &flags, &pBuffer->overlapped, NULL);
            if (recvResult == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                if (error != WSA_IO_PENDING)
                {
                    std::cerr << "[ERROR] WSARecv failed with error: " << error << std::endl;
                    CloseSession(sessionID);
                    delete pBuffer;
                    continue;
                }
                else
                {
                    std::cout << "[DEBUG] WSARecv initiated successfully (IO_PENDING) for session " << sessionID << std::endl;
                }
            }
            else
            {
                // recvResult가 0인 경우, WSARecv가 성공적으로 완료된 것입니다.
                std::cout << "[DEBUG] WSARecv completed immediately for session " << sessionID << std::endl;
            }
        }
        else if (pBuffer->opType == OP_WRITE)
        {
            // 쓰기 작업 완료 처리
            std::cout << "[DEBUG] Send operation completed for session " << sessionID << std::endl;

            // 이전 쓰기 버퍼 해제
            delete pBuffer;

            // **여기서 새로운 WSARecv를 호출함**
            Buffer* pNewBuffer = new Buffer();
            pNewBuffer->opType = OP_READ;
            ZeroMemory(&pNewBuffer->overlapped, sizeof(OVERLAPPED));
            DWORD flags = 0;

            SOCKET clientSocket;
            {
                std::lock_guard<std::mutex> lock(m_SessionMutex);
                clientSocket = m_SessionList[sessionID];
            }

            int recvResult = WSARecv(clientSocket, &pNewBuffer->wsabuf, 1, NULL, &flags, &pNewBuffer->overlapped, NULL);
            if (recvResult == SOCKET_ERROR)
            {
                int error = WSAGetLastError();
                if (error != WSA_IO_PENDING)
                {
                    std::cerr << "[ERROR] WSARecv failed with error: " << error << std::endl;
                    CloseSession(sessionID);
                    delete pBuffer;
                    continue;
                }
                else
                {
                    std::cout << "[DEBUG] WSARecv initiated successfully (IO_PENDING) for session " << sessionID << std::endl;
                }
            }
            else
            {
                std::cout << "[DEBUG] WSARecv completed immediately for session " << sessionID << std::endl;
            }

        }
    }
}


void IOCP_Server::CloseSession(int sessionID)
{
    std::lock_guard<std::mutex> lock(m_SessionMutex);  // 뮤텍스 잠금

    std::cout << "[INFO] Closing session: " << sessionID << ", socket: " << m_SessionList[sessionID] << std::endl;

    if (sessionID >= 0 && sessionID < (int)m_SessionList.size())
    {
        if (m_SessionList[sessionID] != INVALID_SOCKET)
        {
            shutdown(m_SessionList[sessionID], SD_BOTH);
            closesocket(m_SessionList[sessionID]);
            m_SessionList[sessionID] = INVALID_SOCKET;
            std::cout << "[DEBUG] Session " << sessionID << " socket set to INVALID_SOCKET in CloseSession" << std::endl;  // 추가된 로그
        }
        m_SessionQueue.push_back(sessionID);
    }
    else
    {
        std::cerr << "[ERROR] Invalid session ID: " << sessionID << std::endl;
    }
}


void IOCP_Server::Shutdown()
{
    for (size_t i = 0; i < MAX_WORKTHREAD; ++i)
    {
        PostQueuedCompletionStatus(m_hIOCP, 0, 0, NULL);
        WaitForSingleObject(m_WorkThread[i], INFINITE);
        CloseHandle(m_WorkThread[i]);
    }
}

unsigned int WINAPI CallAcceptThread(LPVOID p)
{
    IOCP_Server* pServer = (IOCP_Server*)p;
    pServer->AcceptThread();
    return 0;
}

unsigned int WINAPI CallWorkThread(LPVOID p)
{
    IOCP_Server* pServer = (IOCP_Server*)p;
    pServer->WorkThread();
    return 0;
}
