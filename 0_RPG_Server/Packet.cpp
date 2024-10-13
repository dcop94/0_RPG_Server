#include "Packet.h"
#include "DB_Manager.h"
#include "IOCP_Server.h"

Packet& Packet::Instance()
{
    static Packet instance;
    return instance;
}

void Packet::CheckPacket(std::stringstream& inInputStream, int nSessionID)
{
    std::string packetType;
    std::getline(inInputStream, packetType, ':');

    if (packetType == "LOGIN_REQUEST")
    {
        std::string email, password;
        std::getline(inInputStream, email, ':');
        std::getline(inInputStream, password, ':');

        bool loginSuccess = _DB_Manager.CheckLogin(email, password);

        if (loginSuccess)
        {
            std::cout << "[DEBUG] �α��� ����" << std::endl;
            SendResponse(nSessionID, "LOGIN_SUCCESS");
        }
        else
        {
            std::cout << "[DEBUG] �α��� ����: �̸��� �Ǵ� ��й�ȣ�� Ʋ��" << std::endl;
            SendResponse(nSessionID, "LOGIN_FAIL");
        }
    }
    else if (packetType == "CHECK_EMAIL")
    {
        std::string email;
        std::getline(inInputStream, email, ':');

        bool emailExists = _DB_Manager.CheckEmailExists(email);
        if (emailExists)
        {
            SendResponse(nSessionID, "EMAIL_EXISTS");
        }
        else
        {
            SendResponse(nSessionID, "EMAIL_AVAILABLE");
        }
    }
    else if (packetType == "SIGN_UP")
    {
        std::string email, password;
        std::getline(inInputStream, email, ':');
        std::getline(inInputStream, password, ':');

        bool emailExists = _DB_Manager.CheckEmailExists(email);
        if (emailExists)
        {
            SendResponse(nSessionID, "EMAIL_EXISTS");
            return;
        }

        if (!IsValidPassword(password))
        {
            SendResponse(nSessionID, "INVALID_PASSWORD");
            return;
        }

        std::string accountCode;
        bool signUpSuccess = _DB_Manager.CreateAccount(email, password, accountCode);
        if (signUpSuccess)
        {
            // ���� ���� �� ������ ���� �ڵ带 Ŭ���̾�Ʈ�� ����
            SendResponse(nSessionID, "SIGN_UP_SUCCESS:" + accountCode);
        }
        else
        {
            SendResponse(nSessionID, "SIGN_UP_FAIL");
        }
    }
    else if (packetType == "FIND_ACCOUNT")
    {
        std::string accountCode;
        std::getline(inInputStream, accountCode, ':');

        std::string email;
        bool found = _DB_Manager.GetEmailByAccountCode(accountCode, email);

        if (found)
        {
            SendResponse(nSessionID, "ACCOUNT_FOUND:" + email);
        }
        else
        {
            SendResponse(nSessionID, "ACCOUNT_NOT_FOUND");
        }
    }
    else if (packetType == "CHECK_ACCOUNT")
    {
        std::string accountCode, email;
        std::getline(inInputStream, accountCode, ':');
        std::getline(inInputStream, email, ':');

        bool exists = _DB_Manager.CheckAccountEmailExists(accountCode, email);

        if (exists)
        {
            SendResponse(nSessionID, "ACCOUNT_EXISTS");
        }
        else
        {
            SendResponse(nSessionID, "ACCOUNT_NOT_FOUND");
        }
    }
    else if (packetType == "RESET_PASSWORD")
    {
        std::string email, newPassword;
        std::getline(inInputStream, email, ':');
        std::getline(inInputStream, newPassword, ':');

        bool success = _DB_Manager.UpdatePassword(email, newPassword);

        if (success)
        {
            SendResponse(nSessionID, "PASSWORD_RESET_SUCCESS");
        }
        else
        {
            SendResponse(nSessionID, "PASSWORD_RESET_FAIL");
        }
    }

}


void Packet::SendResponse(int nSessionID, const std::string& message)
{
    Buffer* pBuffer = new Buffer();
    pBuffer->opType = OP_WRITE;
    strcpy_s(pBuffer->buffer, message.c_str());
    pBuffer->wsabuf.buf = pBuffer->buffer;
    pBuffer->wsabuf.len = static_cast<ULONG>(message.length());

    // ���� ���Ͽ� �����ϱ� ���� GetSessionSocket �Լ� ���
    SOCKET sessionSocket = IOCP_Server::Instance().GetSessionSocket(nSessionID);

    if (sessionSocket != INVALID_SOCKET) {
        std::cout << "[DEBUG] Sending response to session " << nSessionID << ", socket: " << sessionSocket << std::endl;
        ZeroMemory(&pBuffer->overlapped, sizeof(OVERLAPPED));  // OVERLAPPED ����ü �ʱ�ȭ
        int result = WSASend(sessionSocket, &pBuffer->wsabuf, 1, NULL, 0, &pBuffer->overlapped, NULL);
        if (result == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if (error != WSA_IO_PENDING)
            {
                std::cerr << "[ERROR] WSASend failed with error: " << error << std::endl;
                IOCP_Server::Instance().CloseSession(nSessionID);
                delete pBuffer;
                return;
            }
            else
            {
                std::cout << "[DEBUG] WSASend initiated successfully (IO_PENDING) for session " << nSessionID << std::endl;
            }
        }
        else
        {
            std::cout << "[DEBUG] WSASend completed immediately for session " << nSessionID << std::endl;
        }

        std::cout << "[DEBUG] ���� ���� �Ϸ�: " << message << std::endl;
    }
    else {
        std::cerr << "[ERROR] ��ȿ���� ���� ���� ����. ������ ������ �� �����ϴ�." << std::endl;
        delete pBuffer;
    }
}

bool Packet::IsValidPassword(const std::string& password)
{
    if (password.length() < 8 || password.length() > 15)
        return false;

    bool hasLetter = false;
    bool hasDigit = false;
    bool hasSymbol = false;

    for (char c : password)
    {
        if (isalpha(c))
            hasLetter = true;
        else if (isdigit(c))
            hasDigit = true;
        else if (ispunct(c) || iscntrl(c) || isspace(c))
            hasSymbol = true;
        else if (iscntrl(c) || isspace(c))
            return false; // ���� ���ڳ� ������ ������� ����
        else
            hasSymbol = true; // ��Ÿ Ư������ ó��
    }

    return hasLetter && hasDigit && hasSymbol;
}


