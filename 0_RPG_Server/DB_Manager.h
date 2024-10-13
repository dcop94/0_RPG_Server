#pragma once
#include "stdfx.h"

class DB_Manager
{
public:
	static DB_Manager& Instance();
	
	DB_Manager();
	~DB_Manager();

	bool CheckDBConnection();
	bool CheckEmailExists(const std::string& email);
	bool CheckPassword(const std::string& email, const std::string& password);
	bool CheckLogin(const std::string& email, const std::string& password);
	bool CheckAccountCodeExists(const std::string& accountCode);
	bool CheckAccountEmailExists(const std::string& accountCode, const std::string& email);

	bool CreateAccount(const std::string& email, const std::string& password, std::string& outAccountCode);
	bool UpdatePassword(const std::string& email, const std::string& newPassword);

	bool GetEmailByAccountCode(const std::string& accountCode, std::string& outemail);
	
	sql::Connection* GetConnection();

private:
	sql::Connection* connection;
	std::string GenerateAccountCode();

};