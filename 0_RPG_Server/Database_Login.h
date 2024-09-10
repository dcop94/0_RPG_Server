#pragma once
#ifndef DATABASE_LOGIN_H
#define DATABASE_LOGIN_H

#include "Database_Server.h"

class Database_login
{
public:
	Database_login(Database_Server* dbserver);

	// 이메일 유효성 검사
	bool validateEmail(const string& use_email);

	// 비밀번호 유효성 검사
	bool validatePassword(const string& password);

	// 로그인 기능 (이메일, 비밀번호로 로그인)
	bool verifyLogin(const string& use_email, const string& password);

	// 회원가입 기능
	bool registerUser(const string& use_email, const string& password);

	// 계정 찾기 기능
	string findAccount(const string& accountCode);

	// 비밀번호 찾기 기능
	string findPassword(const string& use_email, const string& accountCode);

private:
	// 고유 계정 코드 생성
	string geneAccountCode();

	Database_Server* dbserver; // DB 공통 연결

};

#endif // !DATABASE_LOGIN_H

