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

	// ĳ���� ���� ���� Ȯ��
	bool isCharacter(const string& accountCode);

	// ĳ���� ����
	bool createCharacter(const CharacterData& charData);

	// ĳ���� ���� ������Ʈ (��ġ, ����, ����ġ, ü��, ����)
	bool updateCharacterInfo(const CharacterData& charData);

	// ĳ���� ���� ��������
	bool getCharacterInfo(const string& accountCode, CharacterData& charData);

private:
	// ���� ���� �ڵ� ����
	Database_Server* dbserver; // DB ���� ����

};

#endif // !DATABASE_CHARACTER_H
