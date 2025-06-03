#include "ContentsResource.h"
#include "Sector.h"
#include "Msg/Message.h"
#include "ContentsThread/ContentsFunc.h"

std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
std::mutex SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
int sectorXRange = dfRANGE_MOVE_RIGHT / SECTOR_RATIO;
int sectorYRange = dfRANGE_MOVE_BOTTOM / SECTOR_RATIO;

extern Player* g_PlayerArr;
extern CLanServer* ntServer;

bool SyncSector(ULONG64 UserId, int oldSectorX, int oldSectorY)
{

	int playerIndex = ntServer->GetIndex(UserId);
	int currentSectorX = g_PlayerArr[playerIndex].GetX();
	int currentSectorY = g_PlayerArr[playerIndex].GetY();

	
	SectorLockByIndexOrder(oldSectorX, oldSectorY, currentSectorX, currentSectorY);

	size_t debugSize = Sector[oldSectorX][oldSectorY].size();
	Sector[oldSectorX][oldSectorY].remove(&g_PlayerArr[playerIndex]);
	if (debugSize == Sector[oldSectorX][oldSectorY].size())
	{
		__debugbreak();
		return false;
	}

	Sector[currentSectorX][currentSectorY].push_back(&g_PlayerArr[playerIndex]);

	SectorUnlockByIndexOrder(oldSectorX, oldSectorY, currentSectorX, currentSectorY);

	return true;
}






bool CheckSector(ULONG64 UserId)
{
#ifdef __DEBUG__
	int playerIndex = ntServer->GetIndex(UserId);
	int localX;
	int localY;

	localX = g_PlayerArr[playerIndex].GetX();
	localY = g_PlayerArr[playerIndex].GetY();

	SectorLock[localX / SECTOR_RATIO][localY / SECTOR_RATIO].lock();
	auto searchIt = std::find(Sector[localX / SECTOR_RATIO][localY / SECTOR_RATIO].begin(),
		Sector[localX / SECTOR_RATIO][localY / SECTOR_RATIO].end(),
		&g_PlayerArr[playerIndex]);
	if (searchIt == Sector[localX / SECTOR_RATIO][localY / SECTOR_RATIO].end())
	{
		__debugbreak();
		return false;
	}
	SectorLock[localX / SECTOR_RATIO][localY / SECTOR_RATIO].unlock();

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