#pragma once
#ifndef DATABASE_LOGIN_H
#define DATABASE_LOGIN_H

#include "Database_Server.h"

class Database_login
{
public:
	Database_login(Database_Server* dbserver);

	// �̸��� ��ȿ�� �˻�
	bool validateEmail(const string& use_email);

	// ��й�ȣ ��ȿ�� �˻�
	bool validatePassword(const string& password);

	// �α��� ��� (�̸���, ��й�ȣ�� �α���)
	bool verifyLogin(const string& use_email, const string& password);

	// ȸ������ ���
	bool registerUser(const string& use_email, const string& password);

	// ���� ã�� ���
	string findAccount(const string& accountCode);

	// ��й�ȣ ã�� ���
	string findPassword(const string& use_email, const string& accountCode);

private:
	// ���� ���� �ڵ� ����
	string geneAccountCode();

	Database_Server* dbserver; // DB ���� ����

};

#endif // !DATABASE_LOGIN_H

