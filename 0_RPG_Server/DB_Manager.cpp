#include "DB_Manager.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>

DB_Manager& DB_Manager::Instance()
{
    static DB_Manager instance;
    return instance;
}

DB_Manager::DB_Manager()
{
    try
    {
        sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
        connection = driver->connect("tcp://127.0.0.1:3306", "user", "1234");  // MySQL DB ����
        connection->setSchema("zrpg_db");  // DB ��Ű�� ����
        std::cout << "[INFO] DB ���� ����" << std::endl;  // DB ���� ���� �α�
    
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] DB ���� ����: " << e.what() << std::endl;
        connection = nullptr;
    }
}

DB_Manager::~DB_Manager() 
{
    if (connection != nullptr)
    {
        connection->close();
        delete connection;
    }
}

bool DB_Manager::CheckDBConnection()
{
    try {
        sql::Connection* conn = GetConnection();
        if (conn != nullptr) {
            std::cout << "[INFO] DB ���� ����" << std::endl;  // DB ���� ���� �α�
            return true;
        }
        else {
            std::cout << "[ERROR] DB ���� ����: ���� ��ü�� nullptr�Դϴ�." << std::endl;
            return false;
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "[ERROR] DB ���� Ȯ�� ����: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CheckEmailExists(const std::string& email)
{
    try
    {
        std::cout << "[DEBUG] �̸��� ��ȸ ���� ����: " << email << std::endl;

        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT COUNT(*) FROM zrpg_db.users WHERE use_email = ?");
        pstmt->setString(1, email);
        sql::ResultSet* res = pstmt->executeQuery();

        bool exists = false;
        if (res->next()) {
            exists = res->getInt(1) > 0;
            std::cout << "[DEBUG] �̸��� ���� ����: " << exists << std::endl;  // �߰�: �̸��� ���� ���� �α�
        }

        delete res;
        delete pstmt;

        return exists;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] �̸��� ��ȸ ����: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CheckPassword(const std::string& email, const std::string& password)
{
    try
    {
        std::cout << "[DEBUG] ��й�ȣ ��ȸ ���� ����: " << email << std::endl;

        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT use_pwd FROM zrpg_db.users WHERE use_email = ?");
        pstmt->setString(1, email);
        sql::ResultSet* res = pstmt->executeQuery();

        std::string dbPassword;
        bool match = false;

        if (res->next()) {
            dbPassword = res->getString("use_pwd");
            std::cout << "[DEBUG] DB ��й�ȣ: " << dbPassword << std::endl;  // DB���� ������ ��й�ȣ �α� ���
            std::cout << "[DEBUG] �Է� ��й�ȣ: " << password << std::endl;  // �Էµ� ��й�ȣ �α� ���
            match = (dbPassword == password);  // �Էµ� ��й�ȣ�� DB ��й�ȣ�� ��
            std::cout << "[DEBUG] ��й�ȣ ��ġ ����: " << match << std::endl;  // �߰�: ��ġ ���� Ȯ�� �α� ���
        }

        delete res;
        delete pstmt;

        return match;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] ��й�ȣ ��ȸ ����: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CheckLogin(const std::string& email, const std::string& password)
{
    try
    {
        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT COUNT(*) FROM users WHERE use_email = ? AND use_pwd = ?");
        pstmt->setString(1, email);
        pstmt->setString(2, password);
        sql::ResultSet* res = pstmt->executeQuery();

        bool loginSuccess = false;
        if (res->next())
        {
            loginSuccess = res->getInt(1) > 0;
        }

        delete res;
        delete pstmt;

        return loginSuccess;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] �α��� �˻� ����: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CreateAccount(const std::string& email, const std::string& password, std::string& outAccountCode)
{
    try
    {
        // ���� �ڵ� ���� �� �ߺ� �˻�
        std::string accountCode;
        do
        {
            accountCode = GenerateAccountCode();
        } while (CheckAccountCodeExists(accountCode));

        // ���� ����
        sql::PreparedStatement* pstmt = connection->prepareStatement("INSERT INTO users (use_email, use_pwd, account_code, regi_date) VALUES (?, ?, ?, NOW())");
        pstmt->setString(1, email);
        pstmt->setString(2, password);
        pstmt->setString(3, accountCode);
        pstmt->executeUpdate();

        delete pstmt;

        outAccountCode = accountCode; // ������ ���� �ڵ带 ��ȯ

        return true;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] ���� ���� ����: " << e.what() << std::endl;
        return false;
    }
}


bool DB_Manager::CheckAccountCodeExists(const std::string& accountCode)
{
    try
    {
        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT COUNT(*) FROM users WHERE account_code = ?");
        pstmt->setString(1, accountCode);
        sql::ResultSet* res = pstmt->executeQuery();

        bool exists = false;
        if (res->next())
        {
            exists = res->getInt(1) > 0;
        }

        delete res;
        delete pstmt;

        return exists;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] ���� �ڵ� �ߺ� �˻� ����: " << e.what() << std::endl;
        return false;
    }
}


std::string DB_Manager::GenerateAccountCode()
{
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digits = "0123456789";

    // �빮�� 3�ڸ� ����
    std::string part1;
    for (int i = 0; i < 3; ++i)
    {
        part1 += chars[rand() % chars.size()];
    }

    // ���� 3�ڸ� ����
    std::string part2;
    for (int i = 0; i < 3; ++i)
    {
        part2 += digits[rand() % digits.size()];
    }

    // �ҹ��� 3�ڸ� ����
    std::string part3;
    for (int i = 0; i < 3; ++i)
    {
        part3 += std::tolower(chars[rand() % chars.size()]);
    }

    // ���Ŀ� �°� ����
    std::string accountCode = part1 + "-" + part2 + "-" + part3;

    return accountCode;
}

bool DB_Manager::GetEmailByAccountCode(const std::string& accountCode, std::string& outEmail)
{
    try
    {
        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT use_email FROM users WHERE account_code = ?");
        pstmt->setString(1, accountCode);
        sql::ResultSet* res = pstmt->executeQuery();

        if (res->next())
        {
            outEmail = res->getString("use_email");
            delete res;
            delete pstmt;
            return true;
        }
        else
        {
            delete res;
            delete pstmt;
            return false;
        }
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] �̸��� ��ȸ ����: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CheckAccountEmailExists(const std::string& accountCode, const std::string& email)
{
    try
    {
        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT COUNT(*) FROM users WHERE account_code = ? AND use_email = ?");
        pstmt->setString(1, accountCode);
        pstmt->setString(2, email);
        sql::ResultSet* res = pstmt->executeQuery();

        bool exists = false;
        if (res->next())
        {
            exists = res->getInt(1) > 0;
        }

        delete res;
        delete pstmt;

        return exists;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] ���� Ȯ�� ����: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::UpdatePassword(const std::string& email, const std::string& newPassword)
{
    try
    {
        sql::PreparedStatement* pstmt = connection->prepareStatement("UPDATE users SET use_pwd = ? WHERE use_email = ?");
        pstmt->setString(1, newPassword);
        pstmt->setString(2, email);
        pstmt->executeUpdate();

        delete pstmt;

        return true;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] ��й�ȣ ������Ʈ ����: " << e.what() << std::endl;
        return false;
    }
}

sql::Connection* DB_Manager::GetConnection()
{
    return connection;
}
