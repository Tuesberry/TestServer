#pragma once
#include "pch.h"
#include "ServerSession.h"
#include "Utils/BufferHelper.h"
#include "ServerPacketHandler.h"
#include "Room.h"

ServerSession::ServerSession()
    : m_connectClientId(0)
    , m_moveTime(0)
{
}

ServerSession::~ServerSession()
{
    //cout << "~ServerSession" << endl;
}

void ServerSession::OnRecvPacket(BYTE* buffer, int len)
{
    bool result = ServerPacketHandler::HandlePacket(
        static_pointer_cast<ServerSession>(shared_from_this()), buffer, len);
}

void ServerSession::OnSend(int len)
{
    //cout << "OnSend Len = " << len << endl;
}

void ServerSession::OnDisconnected()
{
    gRoom.Logout(m_connectClientId);
}

void ServerSession::SendMoveMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_MOVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_MOVE pktMove;
    pktMove.header.id = PROTO_ID::S2C_MOVE;
    pktMove.header.size = sizeof(PKT_S2C_MOVE);
    pktMove.id = targetId;
    pktMove.x = x;
    pktMove.y = y;
    pktMove.moveTime = NULL;

    if (targetId == m_connectClientId)
    {
        // �ڱ� �ڽ��� move message�� ���
        pktMove.moveTime = m_moveTime;
    }

    bw.Write(&pktMove, sizeof(PKT_S2C_MOVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_MOVE));
    Send(sendBuffer);
}

void ServerSession::SendEnterMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_ENTER));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_ENTER pktEnter;
    pktEnter.header.id = PROTO_ID::S2C_ENTER;
    pktEnter.header.size = sizeof(PKT_S2C_ENTER);
    pktEnter.id = targetId;
    pktEnter.x = x;
    pktEnter.y = y;

    bw.Write(&pktEnter, sizeof(PKT_S2C_ENTER));

    sendBuffer->OnWrite(sizeof(PKT_S2C_ENTER));
    Send(sendBuffer);
}

void ServerSession::SendLeaveMsg(int targetId)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LEAVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LEAVE pktLeave;
    pktLeave.header.id = PROTO_ID::S2C_LEAVE;
    pktLeave.header.size = sizeof(PKT_S2C_LEAVE);
    pktLeave.id = targetId;

    bw.Write(&pktLeave, sizeof(PKT_S2C_LEAVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LEAVE));
    Send(sendBuffer);
}