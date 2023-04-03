#pragma once
#include "pch.h"
#include "CoreCommon.h"

class ClientSession;

class ClientPacketHandler
{
public:
	static bool HandlePacket(shared_ptr<ClientSession> session, BYTE* buffer, int len);
private:
	static bool Handle_S2C_MOVE(shared_ptr<ClientSession> session, BYTE* buffer, int len);
	static bool Handle_S2C_ENTER(shared_ptr<ClientSession> session, BYTE* buffer, int len);
	static bool Handle_S2C_LEAVE(shared_ptr<ClientSession> session, BYTE* buffer, int len);
};