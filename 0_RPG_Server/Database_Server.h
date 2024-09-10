#pragma once
#ifndef DATABASE_SERVER_H
#define DATABASE_SERVER_H

#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <string>
#include <queue>


using namespace std;

struct Database_Info
{
	string host;
	string user;
	string password;
	string schema;
};

// �����ͺ��̽� ���� (����, ����)
class Database_Server
{	
public:
	// ������ : DB���� ���� �ʱ�ȭ
	Database_Server(const Database_Info& db_info);

	// �Ҹ��� : DB ���� ����
	~Database_Server();

	sql::Connection* getConnection(); 
	void db_disconnect(); // DB ���� ����

	

private:
	sql::Driver* driver; // MySQL ����̹�
	sql::Connection* connection; // MySQL ����
	bool db_connect(); // DB ����
	Database_Info db_info;


};

#endif // !DATABASE_SERVER_H
