#include "Database_Server.h"
#include <iostream>
#include <cppconn/exception.h>


// 예외처리 등 확인하기

///////////////////////////////////
//// 데이터베이스 전체 관리 구현
///////////////////////////////////

Database_Server::Database_Server(const Database_Info& db_info) : db_info(db_info), connection(nullptr), driver(nullptr)
{
	db_connect();
}

Database_Server::~Database_Server()
{
	db_disconnect();
}

bool Database_Server::db_connect()
{
	try
	{
		driver = get_driver_instance();
		connection = driver->connect(db_info.host, db_info.user, db_info.password);
		connection->setSchema(db_info.schema);
		return true;
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : DB 연결이 실패 되었습니다." << e.what() << endl;
		return false;
	}

}

sql::Connection* Database_Server::getConnection()
{
	return connection;
}

void Database_Server::db_disconnect()
{
	if (connection)
	{
		delete connection;
		connection = nullptr;
	}
}




