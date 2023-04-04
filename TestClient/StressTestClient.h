#pragma once
#include "CoreCommon.h"
#include "Network/IocpClient.h"

class DelayManager
{
public:
	enum
	{
		CONNECT_DELAY_INIT_MS = 10,
		DELAY_LIMIT_MS = 100,
	};

	DelayManager();
	DelayManager(const DelayManager& other) = delete;
	DelayManager(DelayManager&& other) = delete;
	DelayManager& operator=(const DelayManager& other) = delete;
	DelayManager& operator=(DelayManager&& other) = delete;
	~DelayManager() = default;

	void UpdateDelay(unsigned int delay);

	void UpdateAvgDelay(unsigned int delay, unsigned int prevDelay);
	void AddNewInAvgDelay(unsigned int delay);
	void DeleteInAvgDelay(unsigned int delay);

	void CheckDelay();

public:
	unsigned int m_connectionDelay;
	unsigned long int m_avgDelay;
	int m_delayCnt;
	bool m_bCanConnect;

	mutex m_updateLock;

	int updateCnt = 0;
};
extern DelayManager gDelayMgr;

class StressTestClient
{
	
public:
	StressTestClient() = delete;
	StressTestClient(shared_ptr<IocpClient> client);

	StressTestClient(const StressTestClient& other) = delete;
	StressTestClient(StressTestClient&& other) = delete;
	StressTestClient& operator=(const StressTestClient& other) = delete;
	StressTestClient& operator=(StressTestClient&& other) = delete;
	~StressTestClient();

	void RunServer();

private:
	void UpdateConnection();

	void InitOutput();
	void UpdateOutput();

	void MoveCursor(int x, int y);

private:
	shared_ptr<IocpClient> m_client;

	//high_resolution_clock::time_point m_connectStartTime;
	//duration<long long, micro> m_lastConnectTime;

	COORD m_initCursor;
};