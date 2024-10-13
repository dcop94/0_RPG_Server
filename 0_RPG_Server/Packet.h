#pragma once
#include "stdfx.h"

class Packet
{
public:
	static Packet& Instance();

	void CheckPacket(std::stringstream& inInputStream, int nSessionID);
	void SendResponse(int nSessionID, const std::string& message);
	bool IsValidPassword(const std::string& password);
};