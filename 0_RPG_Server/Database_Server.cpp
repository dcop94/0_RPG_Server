#include "Database_Server.h"
#include <iostream>
#include <cppconn/exception.h>


// ����ó�� �� Ȯ���ϱ�

///////////////////////////////////
//// �����ͺ��̽� ��ü ���� ����
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
		cerr << "ERROR : DB ������ ���� �Ǿ����ϴ�." << e.what() << endl;
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




