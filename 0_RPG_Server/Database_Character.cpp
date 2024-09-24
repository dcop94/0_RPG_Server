#include "Database_Character.h"
#include <cppconn/prepared_statement.h>

Database_character::Database_character(Database_Server* dbserver) : dbserver(dbserver) {}

///////////////////////////////////
//// 데이터베이스 캐릭터 관련 구현
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
		cerr << "ERROR : 캐릭터가 생성 된 것이 없습니다." << e.what() << endl;
		return false;
	}
}

bool Database_character::createCharacter(const CharacterData& charData)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"INSERT INTO use_characters (account_code, class, nickname, map_location, level, exp, hp, mp)"
			"VALUES(?, ?, ?, ?, ?, ?, ?, ?) "
		);
		pstmt->setString(1, charData.accountCode);
		pstmt->setString(2, charData.Use_Char_Class);
		pstmt->setString(3, charData.Use_Char_Name);
		pstmt->setString(4, to_string(charData.positionX) + "," + to_string(charData.positionY));
		pstmt->setInt(5, charData.Use_level);
		pstmt->setDouble(6, charData.Use_exp);
		pstmt->setInt(7, charData.Use_hp);
		pstmt->setInt(8, charData.Use_mp);
		pstmt->executeUpdate();

		delete pstmt;
		return true;

	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 캐릭터 생성 실패되었습니다." << e.what() << endl;
		return false;
	}
}

bool Database_character::updateCharacterInfo(const CharacterData& charData)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"UPDATE use_characters SET map_location = ?, level = ?, exp = ?, hp = ?, mp = ? WHERE account_code = ?");
			
		pstmt->setString(1, to_string(charData.positionX) + "," + to_string(charData.positionY));
		pstmt->setInt(2, charData.Use_level);
		pstmt->setDouble(3, charData.Use_exp);
		pstmt->setInt(4, charData.Use_hp);
		pstmt->setInt(5, charData.Use_mp);
		pstmt->setString(6, charData.accountCode);
		pstmt->executeUpdate();

		delete pstmt;
		return true;
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 캐릭터 업데이트가 실패되었습니다." << e.what() << endl;
		return false;
	}
}

bool Database_character::getCharacterInfo(const string& accountCode, CharacterData& charData)
{
	try
	{
		sql::PreparedStatement* pstmt = dbserver->getConnection()->prepareStatement(
			"SELECT class, nickname, map_location, level, exp, hp, mp FROM use_characters WHERE account_code = ?");
		pstmt->setString(1, accountCode);
		sql::ResultSet* res = pstmt->executeQuery();

		if (res->next())
		{
			// 캐릭터 정보 구조체 저장
			charData.Use_Char_Class = res->getString("class");
			charData.Use_Char_Name = res->getString("nickname");
			string mapLocation = res->getString("map_location");
			sscanf_s(mapLocation.c_str(), "%f, %f", &charData.positionX, &charData.positionY);
			charData.Use_level = res->getInt("level");
			charData.Use_exp = res->getDouble("exp");
			charData.Use_hp = res->getInt("hp");
			charData.Use_mp = res->getInt("mp");

			delete res;
			delete pstmt;
			return true;
		}

		delete res;
		delete pstmt;
		return false;
	}
	catch (sql::SQLException& e)
	{
		cerr << "ERROR : 캐릭터 정보 업데이트 오류 입니다." << e.what() << endl;
		return false;
	}
}