#include "stdfx.h"
#include "IOCP_Server.h"

int main()
{
    // ���� �õ� �ʱ�ȭ
    srand(static_cast<unsigned int>(time(NULL)));

    IOCP_Server& iocpServer = IOCP_Server::Instance();

    // DB ���� ���� Ȯ��
    if (!_DB_Manager.CheckDBConnection())
    {
        std::cerr << "[ERROR] DB ���� Ȯ�� ����, ���� ����" << std::endl;
        return -1;  // DB ���� ���� �� ���� ����
    }

    if (!iocpServer.Initialiize())
    {
        std::cerr << "[ERROR] ���� �ʱ�ȭ ����" << std::endl;
        return -1;
    }

    std::cout << "[INFO] ���� ���� ��" << std::endl;

    iocpServer.Run();  // Ŭ���̾�Ʈ ���� �� ��� ����

    return 0;
}
