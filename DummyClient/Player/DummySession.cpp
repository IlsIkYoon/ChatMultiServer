#include "DummySession.h"
#include "Msg/RandStringManager.h"
#include "Network/ThreadNetworkManager.h"

extern CRandStringManager g_RandStringManager;

DummySession* g_DummySessionArr;
extern std::list<DummySession*> g_Sector[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern std::mutex g_SectorLock[dfRANGE_MOVE_RIGHT / SECTOR_RATIO][dfRANGE_MOVE_BOTTOM / SECTOR_RATIO];
extern int sectorXRange;
extern int sectorYRange;
					   


DummySession::DummySession()
{
	_x = 0;
	_y = 0;
	_direction = 0;
	_move = false;
	dummyStatus = static_cast<long>(DummySession::DummyStatus::Idle);
	allMsgRecvd = true;
	for (int i = 0; i < STRINGCOUNT; i++)
	{
		stringCounter[g_RandStringManager.randStringArr[i]] = 0;
	}
	_created = false;


}

void DummySession::Dummy_Clear()
{
	int sectorX = _x / SECTOR_RATIO;
	int sectorY = _y / SECTOR_RATIO; 

	g_SectorLock[sectorX][sectorY].lock();
	size_t debugSize = g_Sector[sectorX][sectorY].size();

	g_Sector[sectorX][sectorY].remove(this);
	if (debugSize == g_Sector[sectorX][sectorY].size())
	{
		__debugbreak();
	}

	g_SectorLock[sectorX][sectorY].unlock();


	_sessionLock.lock();
	//clear();
	_ID._ulong64 = 0;

	_x = 0;
	_y = 0;
	_direction = 0;
	_move = false;

	dummyStatus = static_cast<long>(DummySession::DummyStatus::Created);
	allMsgRecvd = true;
	for (int i = 0; i < STRINGCOUNT; i++)
	{
		stringCounter[g_RandStringManager.randStringArr[i]] = 0;
	}

	if (_socket != NULL)
	{
		closesocket(_socket);
	}
	_recvQ.BufferClear();
	_sendQ.BufferClear();
	_sessionLock.unlock();
	return;
}

bool DummySession::Move(DWORD fixedDeltaTime)
{
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


	//섹터 이동..
	
	SyncSector(this, oldX, oldY);


	return true;
}



bool SyncSector(DummySession* session, int beforeX, int beforeY)
{
	
	int playerX = session->_x;
	int playerY = session->_y;

	int beforeSectorX = beforeX / SECTOR_RATIO;
	int beforeSectorY = beforeY / SECTOR_RATIO;

	int SectorX = playerX / SECTOR_RATIO;
	int SectorY = playerY / SECTOR_RATIO;
	
	g_SectorLock[beforeSectorX][beforeSectorY].lock();

	size_t debugSize = g_Sector[beforeSectorX][beforeSectorY].size();
	g_Sector[beforeSectorX][beforeSectorY].remove(session);

	if (debugSize == g_Sector[beforeSectorX][beforeSectorY].size())
	{
		__debugbreak();
		return false;
	}
	g_SectorLock[beforeSectorX][beforeSectorY].unlock();

	g_SectorLock[SectorX][SectorY].lock();
	g_Sector[SectorX][SectorY].push_back(session);
	g_SectorLock[SectorX][SectorY].unlock();
	return true;
}

bool DummySession::SendBufferEnque(char* buffer, int size, unsigned int* enqueResult)
{
	bool enqueRetval;

	enqueRetval = _sendQ.Enqueue(buffer, size, enqueResult);
	if (enqueRetval == false)
	{
		__debugbreak();
	}
	if ((unsigned int)size != *enqueResult)
	{
		__debugbreak();
	}


	return true;
}

bool DummySession::RecvBufferEnque(char* buffer, int size, unsigned int* enqueResult)
{
	bool enqueRetval;

	enqueRetval = _recvQ.Enqueue(buffer, size, enqueResult);
	if (enqueRetval == false)
	{
		__debugbreak();
	}
	if ((unsigned int)size != *enqueResult)
	{
		__debugbreak();
	}


	return true;
}


bool DummySession::SendBufferDeque(char* buffer, int size, unsigned int* dequeResult)
{
	bool dequeRetval;

	if (size == 0)
	{
		return false;
	}

	dequeRetval = _sendQ.Dequeue(buffer, size, dequeResult);
	if (dequeRetval == false)
	{
		__debugbreak();
	}
	if ((unsigned int)size != *dequeResult)
	{
		__debugbreak();
	}


	return true;

}
bool DummySession::RecvBufferDeque(char* buffer, int size, unsigned int* dequeResult)
{
	bool dequeRetval;

	if (size == 0)
	{
		return false;
	}

	dequeRetval = _recvQ.Dequeue(buffer, size, dequeResult);
	if (dequeRetval == false)
	{
		__debugbreak();
	}
	if ((unsigned int)size != *dequeResult)
	{
		__debugbreak();
	}


	return true;


}