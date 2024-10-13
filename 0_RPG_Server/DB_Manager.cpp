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
        connection = driver->connect("tcp://127.0.0.1:3306", "user", "1234");  // MySQL DB 연결
        connection->setSchema("zrpg_db");  // DB 스키마 설정
        std::cout << "[INFO] DB 연결 성공" << std::endl;  // DB 연결 성공 로그
    
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] DB 연결 실패: " << e.what() << std::endl;
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
            std::cout << "[INFO] DB 연결 성공" << std::endl;  // DB 연결 성공 로그
            return true;
        }
        else {
            std::cout << "[ERROR] DB 연결 실패: 연결 객체가 nullptr입니다." << std::endl;
            return false;
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "[ERROR] DB 연결 확인 실패: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CheckEmailExists(const std::string& email)
{
    try
    {
        std::cout << "[DEBUG] 이메일 조회 쿼리 실행: " << email << std::endl;

        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT COUNT(*) FROM zrpg_db.users WHERE use_email = ?");
        pstmt->setString(1, email);
        sql::ResultSet* res = pstmt->executeQuery();

        bool exists = false;
        if (res->next()) {
            exists = res->getInt(1) > 0;
            std::cout << "[DEBUG] 이메일 존재 여부: " << exists << std::endl;  // 추가: 이메일 존재 여부 로그
        }

        delete res;
        delete pstmt;

        return exists;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] 이메일 조회 실패: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CheckPassword(const std::string& email, const std::string& password)
{
    try
    {
        std::cout << "[DEBUG] 비밀번호 조회 쿼리 실행: " << email << std::endl;

        sql::PreparedStatement* pstmt = connection->prepareStatement("SELECT use_pwd FROM zrpg_db.users WHERE use_email = ?");
        pstmt->setString(1, email);
        sql::ResultSet* res = pstmt->executeQuery();

        std::string dbPassword;
        bool match = false;

        if (res->next()) {
            dbPassword = res->getString("use_pwd");
            std::cout << "[DEBUG] DB 비밀번호: " << dbPassword << std::endl;  // DB에서 가져온 비밀번호 로그 출력
            std::cout << "[DEBUG] 입력 비밀번호: " << password << std::endl;  // 입력된 비밀번호 로그 출력
            match = (dbPassword == password);  // 입력된 비밀번호와 DB 비밀번호를 비교
            std::cout << "[DEBUG] 비밀번호 일치 여부: " << match << std::endl;  // 추가: 일치 여부 확인 로그 출력
        }

        delete res;
        delete pstmt;

        return match;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] 비밀번호 조회 실패: " << e.what() << std::endl;
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
        std::cerr << "[ERROR] 로그인 검사 실패: " << e.what() << std::endl;
        return false;
    }
}

bool DB_Manager::CreateAccount(const std::string& email, const std::string& password, std::string& outAccountCode)
{
    try
    {
        // 고유 코드 생성 및 중복 검사
        std::string accountCode;
        do
        {
            accountCode = GenerateAccountCode();
        } while (CheckAccountCodeExists(accountCode));

        // 계정 생성
        sql::PreparedStatement* pstmt = connection->prepareStatement("INSERT INTO users (use_email, use_pwd, account_code, regi_date) VALUES (?, ?, ?, NOW())");
        pstmt->setString(1, email);
        pstmt->setString(2, password);
        pstmt->setString(3, accountCode);
        pstmt->executeUpdate();

        delete pstmt;

        outAccountCode = accountCode; // 생성된 고유 코드를 반환

        return true;
    }
    catch (sql::SQLException& e)
    {
        std::cerr << "[ERROR] 계정 생성 실패: " << e.what() << std::endl;
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
        std::cerr << "[ERROR] 고유 코드 중복 검사 실패: " << e.what() << std::endl;
        return false;
    }
}


std::string DB_Manager::GenerateAccountCode()
{
    const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string digits = "0123456789";

    // 대문자 3자리 생성
    std::string part1;
    for (int i = 0; i < 3; ++i)
    {
        part1 += chars[rand() % chars.size()];
    }

    // 숫자 3자리 생성
    std::string part2;
    for (int i = 0; i < 3; ++i)
    {
        part2 += digits[rand() % digits.size()];
    }

    // 소문자 3자리 생성
    std::string part3;
    for (int i = 0; i < 3; ++i)
    {
        part3 += std::tolower(chars[rand() % chars.size()]);
    }

    // 형식에 맞게 결합
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
        std::cerr << "[ERROR] 이메일 조회 실패: " << e.what() << std::endl;
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
        std::cerr << "[ERROR] 계정 확인 실패: " << e.what() << std::endl;
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
        std::cerr << "[ERROR] 비밀번호 업데이트 실패: " << e.what() << std::endl;
        return false;
    }
}

sql::Connection* DB_Manager::GetConnection()
{
    return connection;
}
