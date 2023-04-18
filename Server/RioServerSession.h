#pragma once

#include "Common.h"

#if IOCP
#else RIO

#include "Network/RioSession.h"

/* --------------------------------------------------------
*	class:		RioServerSession
*	Summary:	server session used for RIO
-------------------------------------------------------- */

class RioServerSession : public RioSession
{
public:
    RioServerSession();
    ~RioServerSession();

    virtual void OnRecvPacket(char* buffer, int len) override;
    virtual void OnSend(int len) override;
    virtual void OnDisconnected() override;

    void SendMoveMsg(int targetId, unsigned short x, unsigned short y);
    void SendEnterMsg(int targetId, unsigned short x, unsigned short y);
    void SendLeaveMsg(int targetId);
    void SendLoginResult(bool result, unsigned short x, unsigned short y);

public:
    int m_connectClientId;
    int m_moveTime;
    int m_loginTime;
    int m_serverProcessTime;
};
#endif // RIO
