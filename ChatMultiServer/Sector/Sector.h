#pragma once
#include "ContentsResource.h"
#include "Player/Player.h"



bool SyncSector(ULONG64 UserId, int oldSectorX, int oldSectorY);




//-------------------------------------------------
// 디버그용. 현재 섹터에 내가 들어있는지를 확인 
//-------------------------------------------------
bool CheckSector(ULONG64 UserId);

//-------------------------------------------------
// 데드락 방지를 위해 낮은 인덱스의 락부터 잡아주는 함수
//-------------------------------------------------
bool SectorLockByIndexOrder(int beforeX, int beforeY, int currentX, int currentY);
//-------------------------------------------------
// 낮은 인덱스의 락부터 풀어주는 함수
//-------------------------------------------------
bool SectorUnlockByIndexOrder(int beforeX, int beforeY, int currentX, int currentY);