#pragma once
#include "ContentsResource.h"
#include "Player/Player.h"



bool SyncSector(ULONG64 UserId, int oldX, int oldY);



//-------------------------------------------------
// 디버그용. 현재 섹터에 내가 들어있는지를 확인 
//-------------------------------------------------
bool CheckSector(ULONG64 UserId);