#include "Sector.h"
#include "Player/DummySession.h"

std::list<DummySession*> g_Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
std::mutex g_SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
int sectorXRange = dfRANGE_MOVE_RIGHT / SECTOR_RATIO;
int sectorYRange = dfRANGE_MOVE_BOTTOM / SECTOR_RATIO;

extern DummySession* g_DummySessionArr;

/*
bool SyncSector(ULONG64 UserId, int beforeX, int beforeY)
{

	int playerIndex = pLib->GetIndex(UserId);
	int playerX = g_PlayerArr[playerIndex].GetX();
	int playerY = g_PlayerArr[playerIndex].GetY();

	int beforeSectorX = beforeX / SECTOR_RATIO;
	int beforeSectorY = beforeY / SECTOR_RATIO;

	int SectorX = playerX / SECTOR_RATIO;
	int SectorY = playerY / SECTOR_RATIO;

	size_t debugSize = g_Sector[beforeSectorX][beforeSectorY].size();
	g_Sector[beforeSectorX][beforeSectorY].remove(&g_PlayerArr[playerIndex]);

	if (debugSize == g_Sector[beforeSectorX][beforeSectorY].size())
	{
		__debugbreak();
		return false;
	}

	g_Sector[SectorX][SectorY].push_back(&g_PlayerArr[playerIndex]);

	return true;
}






bool CheckSector(ULONG64 UserId)
{
#ifdef __DEBUG__
	int playerIndex = pLib->GetIndex(UserId);
	int localX;
	int localY;

	localX = g_PlayerArr[playerIndex].GetX();
	localY = g_PlayerArr[playerIndex].GetY();
	auto searchIt = std::find(g_Sector[localX / SECTOR_RATIO][localY / SECTOR_RATIO].begin(),
		g_Sector[localX / SECTOR_RATIO][localY / SECTOR_RATIO].end(),
		&g_PlayerArr[playerIndex]);
	if (searchIt == g_Sector[localX / SECTOR_RATIO][localY / SECTOR_RATIO].end())
	{
		__debugbreak();
		return false;
	}
#endif
	return true;
}
*/