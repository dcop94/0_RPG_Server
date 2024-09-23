#pragma once
#ifndef DATABASE_SERVER_H
#define DATABASE_SERVER_H

#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>


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
	Database_Server(const Database_Info& db_info, int pool_size);

	// �Ҹ��� : DB ���� ����
	~Database_Server();

	sql::Connection* getConnection(); 
	void releaseConnection(sql::Connection* connection);
	

	

private:
	queue<sql::Connection*> pool;
	sql::Driver* driver; // MySQL ����̹�
	Database_Info db_info;
	mutex pool_mutex;
	condition_variable pool_cond;
	void createConnection(int count);
	bool isConnectionValid(sql::Connection* conn);
	int max_pool_size;

};

#endif // !DATABASE_SERVER_H
