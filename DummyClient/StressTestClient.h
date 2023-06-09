#pragma once

#include "Common.h"

#include "Network/IocpClient.h"
#include "ClientPacketHandler.h"
#include "Thread/RWLock.h"


/* --------------------------------------------------------
*	class:		StressTestClient
*	Summary:	client for stress test
-------------------------------------------------------- */

class StressTestClient
{
public:
	StressTestClient(shared_ptr<IocpClient> client, int clientNum, int threadCnt = thread::hardware_concurrency());

	StressTestClient() = delete;
	StressTestClient(const StressTestClient& other) = delete;
	StressTestClient(StressTestClient&& other) = delete;
	StressTestClient& operator=(const StressTestClient& other) = delete;
	StressTestClient& operator=(StressTestClient&& other) = delete;
	~StressTestClient();

	void RunClient();

private:
	void ConnectToServer();
	void DisconnectFromServer(int idx);
	bool SendToServer(int idx);
	void ResetSendTime();

	void InitOutput();
	void UpdateOutput();
	void TestStopOutput();

	void MoveCursor(int x, int y);

private:
	// iocp client
	shared_ptr<IocpClient> m_client;

	// draw output
	COORD m_initCursor;

	// stress test
	bool m_bRunClient;
	bool m_bDisconnectComplete;

	int m_startTime;

	int m_clientNum;
	int m_threadCnt;
	int m_jobCnt;

	vector<__int64> m_sendTime;
};