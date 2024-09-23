#include "Database_Login.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <random>
#include <sstream>
#include <regex>

///////////////////////////////////
//// 데이터베이스 로그인 관련 구현
///////////////////////////////////

Database_login::Database_login(Database_Server* dbserver) : dbserver(dbserver) {}

// 이메일 유효성 검사 함수
bool Database_login::validateEmail(const string& use_email)
{
	// 이메일 유효성 검사 정규식
	regex emailRegex(R"(([\w\.-]+)@([\w\.-]+)\.([a-z\.]{2,6}))");
	if (!regex_match(use_email, emailRegex))
	{
		cerr << "ERROR : 이메일 오류 " << endl;
		return false;
	}
	return true;
}

// 비밀번호 유효성 검사 함수
bool Database_login::validatePassword(const string& password)
{
	// 비밀번호 입력 자리수 검사
	if (password.length() < 8 || password.length() > 15)
	{
		cerr << "ERROR : 비밀번호 오류 8자 ~ 15자" << endl;
		return false;
	}

	// 영어, 숫자, 특수문자가 최소 1개씩 포함된 패턴, 공백허용 X 
	regex passwordRegex("^(?=.*[A-Za-z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,15}$");
	if (!regex_match(password, passwordRegex))
	{
		cerr << "ERROR : 비밀번호 오류 문자 포함 공백제외" << endl;
		return false;
	}
	return true;
}

bool Database_login::verifyLogin(const string& use_email_input, const string& password_input)
{
	string use_email = use_email_input;
	string password = password_input;

	try
	{
		while (true)
		{
			// 이메일 유효성 검사 및 재입력 대기
			if (!validateEmail(use_email))
			{
				getline(cin, use_email);
				continue;
			}
			// 이메일 존재 여부 확인
			sql::PreparedStatement* emailStmt = dbserver->getConnection()->prepareStatement(
				"SELECT * FROM users WHERE use_email = ? AND password = ?"
			);
			emailStmt->setString(1, use_email);

			sql::ResultSet* emailRes = emailStmt->executeQuery();
			if (!emailRes->next())
			{
				cerr << "ERROR : 존재하지 않는 메일 오류" << endl;
				delete emailRes;
				delete emailStmt;

				getline(cin, use_email); // 잘못된 이메일일 경우 재입력
				continue;
			}

			delete emailRes;
			delete emailStmt;
			break; // 이메일 유효하면 루프 종료
		}
		
		while (true)
		{
			// 비밀번호 유효성 검사 및 재입력 대기
			if (!validatePassword(password))
			{
				getline(cin, password); // 비밀번호 재입력
				continue; // 잘못된 비밀번호이면 루프 반복
			}

			// 비밀번호 확인
			sql::PreparedStatement* passwordStmt = dbserver->getConnection()->prepareStatement(
				"SELECT * FROM users WHERE use_email = ? AND password = ?"
			);
			passwordStmt->setString(1, use_email);
			passwordStmt->setString(2, password);

			sql::ResultSet* passwordRes = passwordStmt->executeQuery();
			if (passwordRes->next())
			{
				// 로그인 성공 시 last_login 업데이트
				sql::PreparedStatement* updateStmt = dbserver->getConnection()->prepareStatement(
					"UPDATE users SET last_login = NOW() WHERE use_email = ?"
				);
				updateStmt->setString(1, use_email);
				updateStmt->executeUpdate();
				delete updateStmt;

				delete passwordRes;
				delete passwordStmt;

				return true; // 로그인 성공
			}
			else
			{
				cerr << "비밀번호 오류 " << endl;
				delete passwordRes;
				delete passwordStmt;

				getline(cin, password); // 잘못된 비밀번호일 경우 재입력
			}
		}

	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 로그인 오류 입니다." << e.what() << endl;
		return false;
	}
}

bool Database_login::registerUser(const string& use_email, const string& password)
{

	if (!validateEmail(use_email))
	{
		return false;
	}

	if (!validatePassword(password))
	{
		return false;
	}

	try
	{
		string accountCode = geneAccountCode();

		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"INSERT INTO users (use_email, password, account_code, registration_date, last_login) VALUES(?, ?, ?, NOW(), NOW())"
		);
		pstmt->setString(1, use_email);
		pstmt->setString(2, password);
		pstmt->setString(3, accountCode);

		pstmt->executeUpdate();
		delete pstmt;
		return true;

	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 회원가입 오류 입니다." << e.what() << endl;
		return false;
	}
}

string Database_login::geneAccountCode()
{
	string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dis(0, characters.size() - 1);

	stringstream ss;
	for (int i = 0; i < 6; i++)
	{
		ss << characters[dis(gen)];
	}

	return ss.str();
}

string Database_login::findAccount(const string& accountCode)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"SELECT use_email FROM users WHERE account_code = ?"
		);

		pstmt->setString(1, accountCode);
		sql::ResultSet* res = pstmt->executeQuery();

		if (res->next())
		{
			string email = res->getString("use_email");
			delete res;
			delete pstmt;
			return email;
		}

		delete res;
		delete pstmt;
		return "";
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 계정 조회 실패" << e.what() << endl;
		return "";
	}
}

string Database_login::findPassword(const string& use_email, const string& accountCode)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"SELECT password FROM users WHERE use_email = ? AND account_code = ?"
		);

		pstmt->setString(1, use_email);
		pstmt->setString(2, accountCode);
		sql::ResultSet* res = pstmt->executeQuery();

		if (res->next())
		{
			string password = res->getString("password");
			delete res;
			delete pstmt;
			return password;
		}

		delete res;
		delete pstmt;
		return "";
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 비밀번호 찾기 실패" << e.what() << endl;
		return "";
	}
}


