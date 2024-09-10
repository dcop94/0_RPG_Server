#include "Database_Character.h"
#include <cppconn/prepared_statement.h>

Database_character::Database_character(Database_Server* dbserver) : dbserver(dbserver) {}

///////////////////////////////////
//// �����ͺ��̽� ĳ���� ���� ����
///////////////////////////////////



bool Database_character::isCharacter(const string& accountCode)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"SELECT COUNT(*) FROM use_characters WHERE account_code = ?"
		);
		pstmt->setString(1, accountCode);
		sql::ResultSet* res = pstmt->executeQuery();

		res->next();
		bool exists = res->getInt(1) > 0;

		delete res;
		delete pstmt;
		return exists;

	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : ĳ���Ͱ� ���� �� ���� �����ϴ�." << e.what() << endl;
		return false;
	}
}

bool Database_character::createCharacter(const string& accountCode, const string& charClass, const string& charName)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"INSERT INTO use_characters (account_code, class, nickname, map_location, level, exp, hp, mp) VALUES(?, ?, ?, 'start_map', 1, 0, 50, 50) "
		);
		pstmt->setString(1, accountCode);
		pstmt->setString(2, charClass);
		pstmt->setString(3, charName);
		pstmt->execute();

		delete pstmt;
		return true;

	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : ĳ���� ���� ���еǾ����ϴ�." << e.what() << endl;
		return false;
	}
}

bool Database_character::updateCharacterInfo(const string& accountCode, const string& mapLocation, int level, int exp, int hp, int mp)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"UPDATE use_characters SET map_location = ?, level = ?, exp = ?, hp = ?, mp = ? WHERE account_code = ?");
		pstmt->setString(1, mapLocation);
		pstmt->setInt(2, level);
		pstmt->setDouble(3, exp);
		pstmt->setInt(4, hp);
		pstmt->setInt(5, mp);
		pstmt->setString(6, accountCode);
		pstmt->executeUpdate();

		delete pstmt;
		return true;
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : ĳ���� ������Ʈ�� ���еǾ����ϴ�." << e.what() << endl;
		return false;
	}
}

bool Database_character::getCharacterInfo(const string& accountCode, CharacterInfo& characterInfo)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"SELECT * FROM use_character WHERE account_code = ?");
		pstmt->setString(1, accountCode);
		sql::ResultSet* res = pstmt->executeQuery();

		if (res->next())
		{
			// ĳ���� ���� ����ü ����
			characterInfo.Use_Char_Class = res->getString("class");
			characterInfo.Use_Char_Name = res->getString("nickname");
			characterInfo.Map_Location = res->getString("map_location");
			characterInfo.Use_level = res->getInt("level");
			characterInfo.Use_exp = res->getDouble("exp");
			characterInfo.Use_hp = res->getInt("hp");
			characterInfo.Use_mp = res->getInt("mp");
		}
		else
		{
			return false;
		}

		delete res;
		delete pstmt;
		return true;
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : ĳ���� ���� ������Ʈ ���� �Դϴ�." << e.what() << endl;
		return false;
	}
}