#include "ContentsResource.h"
#include "Sector.h"
#include "Msg/Message.h"
#include "ContentsThread/ContentsFunc.h"
#include "ContentsThread/ContentsThreadManager.h"

std::list<Player*> Sector[SECTOR_MAX][SECTOR_MAX];
std::mutex SectorLock[SECTOR_MAX][SECTOR_MAX];
int sectorXRange = SECTOR_MAX;
int sectorYRange = SECTOR_MAX;

extern CLanServer* ntServer;
extern CContentsThreadManager contentsManager;

bool SyncSector(ULONG64 UserId, int oldSectorX, int oldSectorY)
{
	Player* localPlayerList;
	int playerIndex;
	int currentSectorX;
	int currentSectorY;

	localPlayerList = contentsManager.playerList->playerArr;

	playerIndex = ntServer->GetIndex(UserId);
	currentSectorX = localPlayerList[playerIndex].sectorX;
	currentSectorY = localPlayerList[playerIndex].sectorY;
	
	SectorLockByIndexOrder(oldSectorX, oldSectorY, currentSectorX, currentSectorY);

	size_t debugSize = Sector[oldSectorX][oldSectorY].size();
	Sector[oldSectorX][oldSectorY].remove(&localPlayerList[playerIndex]);
	if (debugSize == Sector[oldSectorX][oldSectorY].size())
	{
		__debugbreak();
		return false;
	}

	Sector[currentSectorX][currentSectorY].push_back(&localPlayerList[playerIndex]);

	SectorUnlockByIndexOrder(oldSectorX, oldSectorY, currentSectorX, currentSectorY);

	return true;
}






bool CheckSector(ULONG64 UserId)
{
#ifdef __DEBUG__
	Player* localPlayerList;
	int playerIndex;
	int localX;
	int localY;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = ntServer->GetIndex(UserId);
	localX = localPlayerList[playerIndex].sectorX;
	localY = localPlayerList[playerIndex].sectorY;

	SectorLock[localX][localY].lock();
	auto searchIt = std::find(Sector[localX][localY].begin(),
		Sector[localX][localY].end(),
		&localPlayerList[playerIndex]);
	if (searchIt == Sector[localX][localY].end())
	{
		__debugbreak();
		return false;
	}
	SectorLock[localX][localY].unlock();

#endif
	return true;
}



bool SectorLockByIndexOrder(int beforeX, int beforeY, int currentX, int currentY)
{
	//X를 기준으로 정렬
	int firstX;
	int firstY;
	int secondX;
	int secondY;

	if (beforeX < currentX)
	{
		firstX = beforeX;
		firstY = beforeY;
		secondX = currentX;
		secondY = currentY;
	}
	else {
		firstX = currentX;
		firstY = currentY;
		secondX = beforeX;
		secondY = beforeY;

	}

	SectorLock[firstX][firstY].lock();
	SectorLock[secondX][secondY].lock();
	return true;
}


bool SectorUnlockByIndexOrder(int beforeX, int beforeY, int currentX, int currentY)
{

	//X를 기준으로 먼저 정렬 후에 Y를 기준으로 정렬
	int firstX;
	int firstY;
	int secondX;
	int secondY;

	if (beforeX < currentX)
	{
		firstX = beforeX;
		firstY = beforeY;
		secondX = currentX;
		secondY = currentY;
	}
	else {
		firstX = currentX;
		firstY = currentY;
		secondX = beforeX;
		secondY = beforeY;

	}


	SectorLock[firstX][firstY].unlock();
	SectorLock[secondX][secondY].unlock();
	return true;
}