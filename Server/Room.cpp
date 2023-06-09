#include "Room.h"
#include "Network/Packet.h"

Room gRoom;

Room::Room()
	: m_moveCnt(0)
	, m_loginCnt(0)
	, m_zones()
{
	vector<LockSetPlayerRef> vec;
	for (int i = 0; i < MAP_WIDTH / ZONE_WIDTH; i++)
	{
		for (int j = 0; j < MAP_HEIGHT / ZONE_HEIGHT; j++)
		{
			vec.push_back(make_shared<LockUnorderedSet<shared_ptr<Player>>>());
		}
		m_zones.push_back(vec);
		vec.clear();
	}
}

void Room::Login(std::shared_ptr<Player> player)
{
	// insert player
	auto pos = GetPlayerZoneIdx(player);
	m_zones[std::get<0>(pos)][std::get<1>(pos)]->Insert(player);

	m_loginCnt.fetch_add(1);

	// set player state
	player->m_playerState = State::Connected;

	// send login result
	player->m_ownerSession->SendLoginResult(true, player->m_posX, player->m_posY);
}

void Room::Logout(std::shared_ptr<Player> player)
{
	// erase player
	auto pos = GetPlayerZoneIdx(player);
	m_zones[std::get<0>(pos)][std::get<1>(pos)]->Erase(player);

	m_loginCnt.fetch_sub(1);
}

void Room::MovePlayer(std::shared_ptr<Player> player, unsigned short direction)
{
	if (IsValidPlayer(player) == false)
		return;

	// start processing time
	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// save beforePos
	auto beforePos = GetPlayerZoneIdx(player);
	int beforeX = std::get<0>(beforePos);
	int beforeY = std::get<1>(beforePos);

	// update new Position
	UpdatePlayerPosition(player, direction);
	auto newPos = GetPlayerZoneIdx(player);
	int newX = std::get<0>(newPos);
	int newY = std::get<1>(newPos);

	PlayerInfo pInfo{ player->m_posX, player->m_posY };
	m_playersInfo[player->m_playerId] = pInfo;

	// update time check
	int checkTime1 = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();

	// check same zone
	if (beforeX != newX || beforeY != newY)
	{
		// update zone
		m_zones[newX][newY]->Insert(player);
		m_zones[beforeX][beforeY]->Erase(player);
	}

	// update check
	int checkTime2 = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count();
	player->m_ownerSession->m_updatePosTime = checkTime2 - checkTime1;

	// new view list & old view list
	//unordered_set<int> oldViewList = player->GetViewList();
	unordered_set<int> newViewList;

	// synchronization
	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			int zoneX = newX + x;
			int zoneY = newY + y;

			// check map
			if (zoneX < 0 || zoneX >= MAP_WIDTH / ZONE_WIDTH || zoneY < 0 || zoneY >= MAP_HEIGHT / ZONE_HEIGHT)
			{
				continue;
			}

			// iterate
			m_zones[zoneX][zoneY]->ReadLock();
			for (auto iter = m_zones[zoneX][zoneY]->m_set.begin(); iter != m_zones[zoneX][zoneY]->m_set.end(); iter++)
			{
				// check invalid
				if (IsValidPlayer(*iter) == false)
					continue;

				// check same
				if ((*iter)->m_playerId == player->m_playerId)
					continue;

				// check near
				if (!IsNear(player->m_posX, player->m_posY, (*iter)->m_posX, (*iter)->m_posY))
					continue;

				// add in view list
				newViewList.insert((*iter)->m_playerId);

				// send move packet
				// check target viewlist
				if ((*iter)->IsExistInViewList(player->m_playerId))
				{
					// 기존에 존재하던 것
					SendMoveMsg(*iter, player);
				}
				else
				{
					// 새로 추가된 것
					SendEnterMsg(*iter, player);
				}
				// check player viewlist
				if (player->IsExistInViewList((*iter)->m_playerId))
				{
					// 기존에 존재하던 것
					SendMoveMsg(player, *iter);
				}
				{
					// 새로 추가된 것
					SendEnterMsg(player, *iter);
				}
			}
			m_zones[zoneX][zoneY]->ReadUnlock();
		}
	}
	// old view list check
	/*
	unordered_set<int>::iterator viter;
	for (viter = oldViewList.begin(); viter != oldViewList.end(); viter++)
	{
		if (newViewList.count(*viter) == false)
		{
			// oldViewList에는 있는데, newViewlist에는 없음
			{
				m_players.ReadLock();
				targetPlayer = m_players.m_map.find(*viter)->second;
				m_players.ReadUnlock();
			}

			SendLeaveMsg(player, targetPlayer);
			SendLeaveMsg(targetPlayer, player);
		}
	}
	*/
	player->m_ownerSession->m_synchronizePosTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - checkTime2;

	// update player view list
	player->SetViewList(newViewList);

	player->m_ownerSession->m_serverProcessTime = duration_cast<microseconds>(high_resolution_clock::now().time_since_epoch()).count() - player->m_ownerSession->m_serverProcessTime;

	// send player move
	SendMoveMsg(player, player);
}

bool Room::IsNear(unsigned short posX1, unsigned short posY1, unsigned short posX2, unsigned short posY2)
{
	double result = sqrt(pow(posX1 - posX2, 2) + pow(posY1 - posY2, 2));
	if (result < VIEW_DISTANCE)
		return true;
	return false;
}

bool Room::IsValidPlayer(shared_ptr<Player> player)
{
	return player->m_playerState == State::Connected;
}

tuple<int, int> Room::GetPlayerZoneIdx(shared_ptr<Player> player)
{
	return tuple<int, int>(player->m_posX / ZONE_WIDTH, player->m_posY / ZONE_HEIGHT);
}

void Room::SendMoveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false)
		return;
	if (IsValidPlayer(targetPlayer) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendMoveMsg(targetPlayer->m_playerId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void Room::SendEnterMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false || IsValidPlayer(targetPlayer) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendEnterMsg(targetPlayer->m_playerId, targetPlayer->m_posX, targetPlayer->m_posY);
	}
}

void Room::SendLeaveMsg(std::shared_ptr<Player> player, std::shared_ptr<Player> targetPlayer)
{
	if (IsValidPlayer(player) == false)
		return;

	if (player->m_ownerSession != nullptr)
	{
		player->m_ownerSession->SendLeaveMsg(targetPlayer->m_playerId);
	}
}

void Room::UpdatePlayerPosition(shared_ptr<Player> player, int direction)
{
	switch (direction)
	{
	case MOVE_DIRECTION::FRONT:
		if (player->m_posY + 1 >= MAP_HEIGHT)
			player->m_posY = 0;
		else
			player->m_posY += 1;
		break;
	case MOVE_DIRECTION::BACK:
		if (player->m_posY - 1 <= 0)
			player->m_posY = MAP_HEIGHT -1;
		else
			player->m_posY -= 1;
		break;
	case MOVE_DIRECTION::RIGHT:
		if (player->m_posX + 1 >= MAP_WIDTH)
			player->m_posX = 0;
		else
			player->m_posX += 1;
		break;
	case MOVE_DIRECTION::LEFT:
		if (player->m_posX - 1 <= 0)
			player->m_posX = MAP_WIDTH - 1;
		else
			player->m_posX -= 1;
		break;
	default:
		break;
	}
}
