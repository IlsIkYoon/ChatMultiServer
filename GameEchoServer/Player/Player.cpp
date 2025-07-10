#include "Player.h"



void CPlayer::init(ULONG64 ID)
{
	_id = ID;

}
void CPlayer::clear()
{
	_id = 0;
	accountNo = 0;
}


CPlayerManager::CPlayerManager(unsigned int pPlayerMaxCount)
{
	playerArr = new CPlayer[pPlayerMaxCount];
	playerMaxCount = pPlayerMaxCount;

}