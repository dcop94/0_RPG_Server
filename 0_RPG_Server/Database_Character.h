#pragma once
#ifndef DATABASE_CHARACTER_H
#define DATABASE_CHARACTER_H

#include "Database_Server.h"

struct CharacterData
{
	string accountCode = "";
	string Use_Char_Class = "";
	string Use_Char_Name = "";
	float positionX = 0.0f;
	float positionY = 0.0f;
	int Use_level = 0;
	double Use_exp = 0.0;
	int Use_hp = 100;
	int Use_mp = 100;
};

class Database_character
{
public:
	Database_character(Database_Server* dbserver);

	// 캐릭터 생성 여부 확인
	bool isCharacter(const string& accountCode);

	// 캐릭터 생성
	bool createCharacter(const CharacterData& charData);

	// 캐릭터 정보 업데이트 (위치, 레벨, 경험치, 체력, 마나)
	bool updateCharacterInfo(const CharacterData& charData);

	// 캐릭터 정보 가져오기
	bool getCharacterInfo(const string& accountCode, CharacterData& charData);

private:
	// 고유 계정 코드 생성
	Database_Server* dbserver; // DB 공통 연결

};

#endif // !DATABASE_CHARACTER_H
