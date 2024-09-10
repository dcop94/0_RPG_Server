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

// 데이터베이스 관리 (연결, 해제)
class Database_Server
{	
public:
	// 생성자 : DB연결 정보 초기화
	Database_Server(const Database_Info& db_info);

	// 소멸자 : DB 연결 종료
	~Database_Server();

	sql::Connection* getConnection(); 
	void db_disconnect(); // DB 연결 해제

	

private:
	sql::Driver* driver; // MySQL 드라이버
	sql::Connection* connection; // MySQL 연결
	bool db_connect(); // DB 연결
	Database_Info db_info;


};

#endif // !DATABASE_SERVER_H
