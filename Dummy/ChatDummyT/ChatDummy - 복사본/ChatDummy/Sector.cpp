#include "Network.h"
#include "Sector.h"
#include <list>

std::list<Session*> g_Sector[dfSECTOR_MAX_Y][dfSECTOR_MAX_X];

void GetSectorAround(int iSectorX, int iSectorY, st_SECTOR_AROUND* pSectorAround)
{
    pSectorAround->iCount = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int newX = iSectorX + dx;
            int newY = iSectorY + dy;

            if (newX >= 0 && newX < dfSECTOR_MAX_X && newY >= 0 && newY < dfSECTOR_MAX_Y)
            {
                pSectorAround->Around[pSectorAround->iCount++] = { newX, newY };
            }
        }
    }
}

void AddSector(st_SECTOR_POS sectorPos, Session* session)
{
    if (sectorPos.iX >= 0 && sectorPos.iX < dfSECTOR_MAX_X &&
        sectorPos.iY >= 0 && sectorPos.iY < dfSECTOR_MAX_Y)
    {
        auto& lock = g_SectorLock[sectorPos.iY][sectorPos.iX];
        AcquireSRWLockExclusive(&lock);
        g_Sector[sectorPos.iY][sectorPos.iX].emplace_back(session);
        ReleaseSRWLockExclusive(&lock);
    }
}

void RemoveSector(st_SECTOR_POS sectorPos, Session* session)
{
    if (sectorPos.iX >= 0 && sectorPos.iX < dfSECTOR_MAX_X &&
        sectorPos.iY >= 0 && sectorPos.iY < dfSECTOR_MAX_Y)
    {
        auto& sectorList = g_Sector[sectorPos.iY][sectorPos.iX];
        auto& lock = g_SectorLock[sectorPos.iY][sectorPos.iX];
        AcquireSRWLockExclusive(&lock);
        auto it = std::find(sectorList.begin(), sectorList.end(), session);
        if (it != sectorList.end())
        {
            sectorList.erase(it);
        }
        ReleaseSRWLockExclusive(&lock);
        //sectorList.remove(pCharacter);
    }
}

bool Sector_UpdateSession(Session* session)
{
    int newSectorX = session->shX / dfSECTOR_SIZE_X;
    int newSectorY = session->shY / dfSECTOR_SIZE_Y;

    if (newSectorX == session->CurSector.iX && newSectorY == session->CurSector.iY)
    {
        return false;
    }

    RemoveSector(session->CurSector, session);

    session->OldSector = session->CurSector;
    session->CurSector.iX = newSectorX;
    session->CurSector.iY = newSectorY;

    AddSector(session->CurSector, session);

    return true;
}
