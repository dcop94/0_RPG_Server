#pragma once
#ifndef DATABASE_CHARACTER_H
#define DATABASE_CHARACTER_H

#include "Database_Server.h"

class Database_character
{
public:
	Database_character(Database_Server* dbserver);

	struct CharacterInfo
	{
		string Use_Char_Class;
		string Use_Char_Name;
		string Map_Location;
		int Use_level;
		double Use_exp;
		int Use_hp;
		int Use_mp;
	};

	// 캐릭터 생성 여부 확인
	bool isCharacter(const string& accountCode);

	// 캐릭터 생성
	bool createCharacter(const string& accountCode, const string& charClass, const string& charName);

	// 캐릭터 정보 업데이트 (위치, 레벨, 경험치, 체력, 마나)
	bool updateCharacterInfo(const string& accountCode, const string& mapLocation, int level, int exp, int hp, int mp);

	// 캐릭터 정보 가져오기
	bool getCharacterInfo(const string& accountCode, CharacterInfo& characterInfo);

private:
	// 고유 계정 코드 생성
	Database_Server* dbserver; // DB 공통 연결

};

#endif // !DATABASE_CHARACTER_H
