#include "stdfx.h"
#include "IOCP_Server.h"

int main()
{
    // 랜덤 시드 초기화
    srand(static_cast<unsigned int>(time(NULL)));

    IOCP_Server& iocpServer = IOCP_Server::Instance();

    // DB 연결 상태 확인
    if (!_DB_Manager.CheckDBConnection())
    {
        std::cerr << "[ERROR] DB 연결 확인 실패, 서버 종료" << std::endl;
        return -1;  // DB 연결 실패 시 서버 종료
    }

    if (!iocpServer.Initialiize())
    {
        std::cerr << "[ERROR] 서버 초기화 실패" << std::endl;
        return -1;
    }

    std::cout << "[INFO] 서버 시작 중" << std::endl;

    iocpServer.Run();  // 클라이언트 수락 및 통신 시작

    return 0;
}
