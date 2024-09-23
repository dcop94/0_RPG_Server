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

// 데이터베이스 관리 (연결, 해제)
class Database_Server
{	
public:
	// 생성자 : DB연결 정보 초기화
	Database_Server(const Database_Info& db_info, int pool_size);

	// 소멸자 : DB 연결 종료
	~Database_Server();

	sql::Connection* getConnection(); 
	void releaseConnection(sql::Connection* connection);
	

	

private:
	queue<sql::Connection*> pool;
	sql::Driver* driver; // MySQL 드라이버
	Database_Info db_info;
	mutex pool_mutex;
	condition_variable pool_cond;
	void createConnection(int count);
	bool isConnectionValid(sql::Connection* conn);
	int max_pool_size;

};

#endif // !DATABASE_SERVER_H
