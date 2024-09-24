#include "Database_Server.h"
#include <iostream>
#include <cppconn/exception.h>
#include <mysql_driver.h>
#include <mysql_connection.h>


// 예외처리 등 확인하기

///////////////////////////////////
//// 데이터베이스 전체 관리 구현
///////////////////////////////////

Database_Server::Database_Server(const Database_Info& db_info, int pool_size) : db_info(db_info), driver(nullptr), max_pool_size(pool_size)
{
	driver = sql::mysql::get_mysql_driver_instance();
	createConnection(pool_size);
}

Database_Server::~Database_Server()
{
	while (!pool.empty())
	{
		sql::Connection* connection = pool.front();
		pool.pop();
		delete connection;
	}
}

void Database_Server::createConnection(int count)
{
	for (int i = 0; i < count; i++)
	{
		try
		{
			sql::Connection* conn = driver->connect(db_info.host, db_info.user, db_info.password);
			conn->setSchema(db_info.schema);
			pool.push(conn);
		}
		catch (sql::SQLException& e)
		{
			cerr << "ERROR : 연결 생성 실패" << e.what() << endl;
		}
	}

}

bool Database_Server::isConnectionValid(sql::Connection* conn)
{
	try
	{
		if (conn->isClosed())
		{
			cerr << "ERROR : 연결 종료" << endl;
			return false;
		}
		conn->setSchema(db_info.schema);
		return true;
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 연결 확인 실패" << e.what() << endl;
		return false;
	}
}

sql::Connection* Database_Server::getConnection()
{
	unique_lock<mutex> lock(pool_mutex);

	while (pool.empty())
	{
		pool_cond.wait(lock);
	}
	sql::Connection* conn = pool.front();
	pool.pop();

	if (!isConnectionValid(conn))
	{
		delete conn;
		conn = driver->connect(db_info.host, db_info.user, db_info.password);
		conn->setSchema(db_info.schema);
	}

	return conn;
}

void Database_Server::releaseConnection(sql::Connection* connection)
{
	unique_lock<mutex> lock(pool_mutex);
	pool.push(connection);
	pool_cond.notify_one();
}




