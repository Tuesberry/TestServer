#include "IOCPServerSession.h"

#include "Utils/BufferHelper.h"
#include "ServerPacketHandler.h"

#include "Room.h"
//#include "RoomOrigin.h"

#include "Player.h"

#if IOCP

/* --------------------------------------------------------
*	Method:		IocpServerSession::IocpServerSession
*	Summary:	Constructor
-------------------------------------------------------- */
IocpServerSession::IocpServerSession()
    : m_connectClientId(0)
    , m_moveTime(0)
    , m_loginTime(0)
    , m_serverProcessTime(0)
    , m_ownPlayer()
{
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::~IocpServerSession
*	Summary:	Destructor
-------------------------------------------------------- */
IocpServerSession::~IocpServerSession()
{
    if (m_ownPlayer != nullptr)
    {
        if (m_ownPlayer->m_playerState == State::Connected)
        {
            m_ownPlayer->m_playerState = State::Disconnected;
            gRoom.Logout(m_ownPlayer);
        }
    }
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::OnRecvPacket
*	Summary:	called when receive packet
*   Args:       BYTE* buffer
*                   buffer containing received data
*               int len
*                   data length
-------------------------------------------------------- */
void IocpServerSession::OnRecvPacket(BYTE* buffer, int len)
{
    bool result = ServerPacketHandler::HandlePacket(
        static_pointer_cast<IocpServerSession>(shared_from_this()), buffer, len);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::OnSend
*	Summary:	called after sending packet
*   Args:       int len
*                   sending data length
-------------------------------------------------------- */
void IocpServerSession::OnSend(int len)
{
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::OnDisconnected
*	Summary:	called after disconnecting from client
-------------------------------------------------------- */
void IocpServerSession::OnDisconnected()
{
    if (m_ownPlayer == nullptr)
        return;

    if (m_ownPlayer->m_playerState == State::Connected)
    {
        m_ownPlayer->m_playerState = State::Disconnected;
        gRoom.Logout(m_ownPlayer);
    }

    m_ownPlayer = nullptr;
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendMoveMsg
*	Summary:	send move message to client
*   Args:       int targetId
*                   target client id
*               unsigned short x
*                   position x
*               unsigned short y
*                   position y
-------------------------------------------------------- */
void IocpServerSession::SendMoveMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_MOVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_MOVE pktMove;
    pktMove.header.id = PROTO_ID::S2C_MOVE;
    pktMove.header.size = sizeof(PKT_S2C_MOVE);
    pktMove.id = m_connectClientId;
    pktMove.targetId = targetId;
    pktMove.x = x;
    pktMove.y = y;
    pktMove.moveTime = NULL;
    pktMove.processTime = NULL;

    if (targetId == m_connectClientId)
    {
        // 자기 자신의 move message인 경우
        pktMove.moveTime = m_moveTime;
        pktMove.processTime = m_serverProcessTime;
        pktMove.sendTime = m_sendTime;
        pktMove.recvTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
        pktMove.updatePosTime = m_updatePosTime;
        pktMove.synchronizePosTime = m_synchronizePosTime;
    }

    bw.Write(&pktMove, sizeof(PKT_S2C_MOVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_MOVE));
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendEnterMsg
*	Summary:	send enter message to client
*   Args:       int targetId
*                   target client id
*               unsigned short x
*                   position x
*               unsigned short y
*                   position y
-------------------------------------------------------- */
void IocpServerSession::SendEnterMsg(int targetId, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_ENTER));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_ENTER pktEnter;
    pktEnter.header.id = PROTO_ID::S2C_ENTER;
    pktEnter.header.size = sizeof(PKT_S2C_ENTER);
    pktEnter.id = m_connectClientId;
    pktEnter.targetId = targetId;
    pktEnter.x = x;
    pktEnter.y = y;

    bw.Write(&pktEnter, sizeof(PKT_S2C_ENTER));

    sendBuffer->OnWrite(sizeof(PKT_S2C_ENTER));
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendLeaveMsg
*	Summary:	send leave message to client
*   Args:       int targetId
*                   target client id
-------------------------------------------------------- */
void IocpServerSession::SendLeaveMsg(int targetId)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LEAVE));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LEAVE pktLeave;
    pktLeave.header.id = PROTO_ID::S2C_LEAVE;
    pktLeave.header.size = sizeof(PKT_S2C_LEAVE);
    pktLeave.id = m_connectClientId;
    pktLeave.targetId = targetId;

    bw.Write(&pktLeave, sizeof(PKT_S2C_LEAVE));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LEAVE));
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendLeaveMsg
*	Summary:	send leave message to client
*   Args:       bool result
*                   login result
*               unsigned short x
*                   position x
*               unsigned short y
*                   position y
-------------------------------------------------------- */
void IocpServerSession::SendLoginResult(bool result, unsigned short x, unsigned short y)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LOGIN_RESULT));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LOGIN_RESULT pktResult;
    pktResult.header.id = PROTO_ID::LOGIN_RESULT;
    pktResult.header.size = sizeof(PKT_S2C_LOGIN_RESULT);
    pktResult.result = result;
    pktResult.id = m_connectClientId;
    pktResult.x = x;
    pktResult.y = y;
    pktResult.loginTime = m_loginTime;

    bw.Write(&pktResult, sizeof(PKT_S2C_LOGIN_RESULT));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LOGIN_RESULT));
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendLoginResultAdmin
*	Summary:	send login result to admin
*   Args:       bool result
*                   login result
-------------------------------------------------------- */
void IocpServerSession::SendLoginResultAdmin(bool result)
{
    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(sizeof(PKT_S2C_LOGIN_RESULT_ADMIN));
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2C_LOGIN_RESULT_ADMIN pktResult;
    pktResult.header.id = PROTO_ID::LOGIN_RESULT_ADMIN;
    pktResult.header.size = sizeof(PKT_S2C_LOGIN_RESULT_ADMIN);
    pktResult.result = result;
    pktResult.id = m_connectClientId;

    bw.Write(&pktResult, sizeof(PKT_S2C_LOGIN_RESULT_ADMIN));

    sendBuffer->OnWrite(sizeof(PKT_S2C_LOGIN_RESULT_ADMIN));
    Send(sendBuffer);
}

/* --------------------------------------------------------
*	Method:		IocpServerSession::SendPlayersInfo
*	Summary:	send players info to admin
*   Args:       map<int, PlayerInfo> playerInfo
*                   players position info
-------------------------------------------------------- */
void IocpServerSession::SendPlayersInfo(map<int, PlayerInfo> playerInfo)
{
    int bufSize = sizeof(PKT_S2A_PLAYER_INFO) + sizeof(PLAYER_INFO) * playerInfo.size();

    shared_ptr<SendBuffer> sendBuffer = make_shared<SendBuffer>(bufSize);
    BufferWriter bw(sendBuffer->GetData(), sendBuffer->GetFreeSize());

    PKT_S2A_PLAYER_INFO pktInfo;
    pktInfo.header.id = PROTO_ID::S2A_PLAYER_INFO;
    pktInfo.header.size = bufSize;
    pktInfo.playerNum = playerInfo.size();

    bw.Write(&pktInfo, sizeof(PKT_S2A_PLAYER_INFO));

    for (auto player : playerInfo)
    {
        bw << player.first;
        bw << player.second.x;
        bw << player.second.y;
    }
    
    sendBuffer->OnWrite(bufSize);
    Send(sendBuffer);
}

#endif // IOCP