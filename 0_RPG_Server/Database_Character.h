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

	// ĳ���� ���� ���� Ȯ��
	bool isCharacter(const string& accountCode);

	// ĳ���� ����
	bool createCharacter(const string& accountCode, const string& charClass, const string& charName);

	// ĳ���� ���� ������Ʈ (��ġ, ����, ����ġ, ü��, ����)
	bool updateCharacterInfo(const string& accountCode, const string& mapLocation, int level, int exp, int hp, int mp);

	// ĳ���� ���� ��������
	bool getCharacterInfo(const string& accountCode, CharacterInfo& characterInfo);

private:
	// ���� ���� �ڵ� ����
	Database_Server* dbserver; // DB ���� ����

};

#endif // !DATABASE_CHARACTER_H
