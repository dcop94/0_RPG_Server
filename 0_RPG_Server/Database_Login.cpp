#include "Database_Login.h"
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <random>
#include <sstream>
#include <regex>

///////////////////////////////////
//// �����ͺ��̽� �α��� ���� ����
///////////////////////////////////

Database_login::Database_login(Database_Server* dbserver) : dbserver(dbserver) {}

// �̸��� ��ȿ�� �˻� �Լ�
bool Database_login::validateEmail(const string& use_email)
{
	// �̸��� ��ȿ�� �˻� ���Խ�
	regex emailRegex(R"(([\w\.-]+)@([\w\.-]+)\.([a-z\.]{2,6}))");
	if (!regex_match(use_email, emailRegex))
	{
		cerr << "ERROR : �̸��� ���� " << endl;
		return false;
	}
	return true;
}

// ��й�ȣ ��ȿ�� �˻� �Լ�
bool Database_login::validatePassword(const string& password)
{
	// ��й�ȣ �Է� �ڸ��� �˻�
	if (password.length() < 8 || password.length() > 15)
	{
		cerr << "ERROR : ��й�ȣ ���� 8�� ~ 15��" << endl;
		return false;
	}

	// ����, ����, Ư�����ڰ� �ּ� 1���� ���Ե� ����, ������� X 
	regex passwordRegex("^(?=.*[A-Za-z])(?=.*\\d)(?=.*[@$!%*?&])[A-Za-z\\d@$!%*?&]{8,15}$");
	if (!regex_match(password, passwordRegex))
	{
		cerr << "ERROR : ��й�ȣ ���� ���� ���� ��������" << endl;
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
			// �̸��� ��ȿ�� �˻� �� ���Է� ���
			if (!validateEmail(use_email))
			{
				getline(cin, use_email);
				continue;
			}
			// �̸��� ���� ���� Ȯ��
			sql::PreparedStatement* emailStmt = dbserver->getConnection()->prepareStatement(
				"SELECT * FROM users WHERE use_email = ? AND password = ?"
			);
			emailStmt->setString(1, use_email);

			sql::ResultSet* emailRes = emailStmt->executeQuery();
			if (!emailRes->next())
			{
				cerr << "ERROR : �������� �ʴ� ���� ����" << endl;
				delete emailRes;
				delete emailStmt;

				getline(cin, use_email); // �߸��� �̸����� ��� ���Է�
				continue;
			}

			delete emailRes;
			delete emailStmt;
			break; // �̸��� ��ȿ�ϸ� ���� ����
		}
		
		while (true)
		{
			// ��й�ȣ ��ȿ�� �˻� �� ���Է� ���
			if (!validatePassword(password))
			{
				getline(cin, password); // ��й�ȣ ���Է�
				continue; // �߸��� ��й�ȣ�̸� ���� �ݺ�
			}

			// ��й�ȣ Ȯ��
			sql::PreparedStatement* passwordStmt = dbserver->getConnection()->prepareStatement(
				"SELECT * FROM users WHERE use_email = ? AND password = ?"
			);
			passwordStmt->setString(1, use_email);
			passwordStmt->setString(2, password);

			sql::ResultSet* passwordRes = passwordStmt->executeQuery();
			if (passwordRes->next())
			{
				// �α��� ���� �� last_login ������Ʈ
				sql::PreparedStatement* updateStmt = dbserver->getConnection()->prepareStatement(
					"UPDATE users SET last_login = NOW() WHERE use_email = ?"
				);
				updateStmt->setString(1, use_email);
				updateStmt->executeUpdate();
				delete updateStmt;

				delete passwordRes;
				delete passwordStmt;

				return true; // �α��� ����
			}
			else
			{
				cerr << "��й�ȣ ���� " << endl;
				delete passwordRes;
				delete passwordStmt;

				getline(cin, password); // �߸��� ��й�ȣ�� ��� ���Է�
			}
		}

	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : �α��� ���� �Դϴ�." << e.what() << endl;
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
		cerr << "ERROR : ȸ������ ���� �Դϴ�." << e.what() << endl;
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
		cerr << "ERROR : ���� ��ȸ ����" << e.what() << endl;
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
		cerr << "ERROR : ��й�ȣ ã�� ����" << e.what() << endl;
		return "";
	}
}


