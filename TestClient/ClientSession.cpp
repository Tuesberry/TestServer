#pragma once
#include "ClientSession.h"
#include "ClientPacketHandler.h"
#include "Utils/ConsoleOutputManager.h"
#include "Utils/BufferHelper.h"
#include "Network/IocpService.h"

ClientSession::ClientSession()
	: m_sessionID(0)
	, m_posX(0)
	, m_posY(0)
{
}

ClientSession::~ClientSession()
{
	//cout << "~ClientSession" << endl;
}

void ClientSession::OnConnected()
{
	// set sessionID
	m_sessionID = GetService()->GetConnectCnt();

	string temp = "Connected To Server |   ID :  " + to_string(m_sessionID);
	GCoutMgr << temp;

	// sendLogin
	SendLogin();
}

void ClientSession::OnRecvPacket(BYTE* buffer, int len)
{
	bool result = ClientPacketHandler::HandlePacket(
		static_pointer_cast<ClientSession>(shared_from_this()), buffer, len);

	if (result == false)
		Disconnect();

	SendMove();
}

void ClientSession::OnSend(int len)
{
	//cout << "OnSend Len = " << len << endl;
}

void ClientSession::OnDisconnected()
{
	cout << "Disconnected" << endl;
}

void ClientSession::SendLogin()
{
	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_C2S_LOGIN));
	BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

	PKT_C2S_LOGIN pktLogin;
	pktLogin.header.id = PROTO_ID::LOGIN;
	pktLogin.header.size = sizeof(PKT_C2S_LOGIN);
	pktLogin.id = m_sessionID;

	bw.Write(&pktLogin, sizeof(PKT_C2S_LOGIN));

	sendBuffer->OnWrite(sizeof(PKT_C2S_LOGIN));
	Send(sendBuffer);
}

void ClientSession::SendMove()
{
	shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_C2S_MOVE));
	BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

	PKT_C2S_MOVE pktMove;
	pktMove.header.id = PROTO_ID::C2S_MOVE;
	pktMove.header.size = sizeof(PKT_C2S_MOVE);
	pktMove.id = m_sessionID;
	pktMove.direction = rand() % 4;

	bw.Write(&pktMove, sizeof(PKT_C2S_MOVE));

	sendBuffer->OnWrite(sizeof(PKT_C2S_MOVE));
	Send(sendBuffer);
}