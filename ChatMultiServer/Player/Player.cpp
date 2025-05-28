#include "ContentsResource.h"
#include "Player.h"
#include "Msg/ContentsPacket.h"
#include "Sector/Sector.h"
#include "ContentsThread/ContentsFunc.h"
#include "Msg/Message.h"
//-----------------------------------------
// 플레이어 카운팅을 위한 변수
//-----------------------------------------
unsigned long long g_TotalPlayerCreate;
unsigned long long g_PlayerLogInCount;
unsigned long long g_PlayerLogOut;
unsigned long long g_HeartBeatOverCount;

long long g_playerCount;
unsigned long long g_PlayerID;

extern std::stack<int> g_playerIndexStack;
Player* g_PlayerArr;

extern std::list<Player*> Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern std::mutex SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern CLanServer* ntServer;

//-----------------------------------------
// Session은 여유가 있는데 Player가 맥스일때 들어가는 대기열
// 로비 서버가 없는 게임 서버라면 혹시 몰라서 있는 대기열임
//-----------------------------------------
std::queue<ULONG64> g_WaitingPlayerAcceptQ;


bool Player::Move(DWORD fixedDeltaTime) {
	if (_move == false) return false;

	if (_x >= dfRANGE_MOVE_RIGHT || _x < dfRANGE_MOVE_LEFT || _y >= dfRANGE_MOVE_BOTTOM || _y < dfRANGE_MOVE_TOP) return false;


	short deltaX;
	short deltaY;
	int oldX = _x;
	int oldY = _y;

	deltaX = ((short)fixedDeltaTime / FrameSec) * dfSPEED_PLAYER_X;
	deltaY = ((short)fixedDeltaTime / FrameSec) * dfSPEED_PLAYER_Y;



	switch (_direction) {
	case dfPACKET_MOVE_DIR_LL:
	{
		if (_x - deltaX < dfRANGE_MOVE_LEFT) return false;
		_x -= deltaX;;

	}
	break;

	case dfPACKET_MOVE_DIR_LU:
	{
		if (_x - deltaX < dfRANGE_MOVE_LEFT || _y - deltaY < dfRANGE_MOVE_TOP) return false;
		_x -= deltaX;
		_y -= deltaY;

	}

	break;

	case dfPACKET_MOVE_DIR_UU:
	{
		if (_y - deltaY < dfRANGE_MOVE_TOP) return false;
		_y -= deltaY;


	}

	break;

	case dfPACKET_MOVE_DIR_RU:
	{
		if (_x + deltaX >= dfRANGE_MOVE_RIGHT || _y - deltaY < dfRANGE_MOVE_TOP) return false;
		_x += deltaX;
		_y -= deltaY;


	}
	break;

	case dfPACKET_MOVE_DIR_RR:
	{
		if (_x + deltaX >= dfRANGE_MOVE_RIGHT) return false;
		_x += deltaX;


	}
	break;

	case dfPACKET_MOVE_DIR_RD:
	{
		if (_x + deltaX >= dfRANGE_MOVE_RIGHT || _y + deltaY >= dfRANGE_MOVE_BOTTOM) return false;
		_x += deltaX;
		_y += deltaY;


	}
	break;

	case dfPACKET_MOVE_DIR_DD:
	{
		if (_y + deltaY >= dfRANGE_MOVE_BOTTOM) return false;
		_y += deltaY;


	}
	break;

	case dfPACKET_MOVE_DIR_LD:
	{
		if (_x - deltaX < dfRANGE_MOVE_LEFT || _y + deltaY >= dfRANGE_MOVE_BOTTOM) return false;
		_x -= deltaX;
		_y += deltaY;


	}
	break;
	default:
		__debugbreak();
		break;
	}



	if ((_x / SECTOR_RATIO == oldX / SECTOR_RATIO) && (_y / SECTOR_RATIO == oldY / SECTOR_RATIO))
	{
		return true;
	}


	SyncSector(_ID, oldX, oldY);

	return true;
}



bool Player::MoveStart(BYTE Direction, int x, int y) {

	int oldX = _x;
	int oldY = _y;

	int oldSectorX = oldX / SECTOR_RATIO;
	int oldSectorY = oldY / SECTOR_RATIO;

	_direction = Direction;
	_x = x;
	_y = y;
	_move = true;

	int newSectorX = x / SECTOR_RATIO;
	int newSectorY = y / SECTOR_RATIO;

	//섹터 동기화
	if ((newSectorX != oldSectorX) || (newSectorY != oldSectorY))
	{
		SyncSector(_ID, oldX, oldY);
	}


	return true;
}



void Player::MoveStop(BYTE Dir, int x, int y)
{
	int oldX = _x;
	int oldY = _y;

	int oldSectorX = oldX / SECTOR_RATIO;
	int oldSectorY = oldY / SECTOR_RATIO;



	_direction = Dir;
	_x = x;
	_y = y;

	_move = false;

	int newSectorX = x / SECTOR_RATIO;
	int newSectorY = y / SECTOR_RATIO;

	//섹터 동기화
	if ((newSectorX != oldSectorX) || (newSectorY != oldSectorY))
	{
		SyncSector(_ID, oldX, oldY);
	}


}

void Player::Clear()
{
	_status = static_cast<BYTE>(STATUS::DELETED);

}

void Player::Init(ULONG64 sessionID)
{

	_x = rand() % 6400;
	_y = rand() % 6400;
	_direction = (rand() % 2) * 4; //LL == 0, RR == 4


	_move = false;
	_ID = sessionID;
	_status = static_cast<BYTE>(STATUS::ALIVE);
	_timeOut = timeGetTime();

	SectorLock[_x / SECTOR_RATIO][_y / SECTOR_RATIO].lock();
	Sector[_x / SECTOR_RATIO][_y / SECTOR_RATIO].push_back(this);
	SectorLock[_x / SECTOR_RATIO][_y / SECTOR_RATIO].unlock();

}

bool Player::isAlive()
{
	if (_status == static_cast<BYTE>(STATUS::ALIVE))
	{
		return true;
	}

	return false;
}


unsigned long long Player::GetID()
{
	return _ID;
}




void EnqueueWaitingPlayerQ(ULONG64 id)
{
	g_PlayerArr[ntServer->GetIndex(id)]._status = static_cast<BYTE>(Player::STATUS::WAIT_CREATE);
	g_WaitingPlayerAcceptQ.push(id);

	//실제 게임이라면 여기에 대기열로 입장시키는 메세지 전송 로직이 들어가야함
}

bool DequeueWaitingPlayerQ()
{
	ULONG64 playerID;
	unsigned short playerIndex;
	while (g_WaitingPlayerAcceptQ.size() != 0)
	{
		playerID = g_WaitingPlayerAcceptQ.front();
		g_WaitingPlayerAcceptQ.pop();
		playerIndex = ntServer->GetIndex(playerID);
		if (g_PlayerArr[playerIndex]._status == static_cast<BYTE>(Player::STATUS::DELETED))
		{
			continue;
		}
		else if (g_PlayerArr[playerIndex]._status == static_cast<BYTE>(Player::STATUS::ALIVE))
		{
			__debugbreak();
			continue;
		}
		else
		{
			g_PlayerArr[playerIndex].Init(playerID);
			ContentsSendCreatePlayerPacket(playerID);
			return true;
		}

		break;
	}

	return false;
}