#include "pch.h"
#include "Session.h"


ULONG64 Session::SessionIndexUnion::GetID()
{
	ULONG64 retval = 0;
	retval |= (ULONG64)_struct._id._dword;
	retval |= ((ULONG64)_struct._id._word) << 32;

	return retval;
}

void Session::SessionIndexUnion::SetID(ULONG64 ID)
{
	ULONG64 byte4;
	ULONG64 byte2;

	byte4 = ID & 0x00000000ffffffff;
	byte2 = ID & 0x0000ffff00000000;
	byte2 = byte2 >> 32;

	_struct._id._dword = (DWORD)byte4;
	_struct._id._word = (WORD)byte2;
}

unsigned short Session::SessionIndexUnion::GetIndex()
{
	return _struct._idex;
}

void Session::SessionIndexUnion::SetIndex(unsigned short index)
{
	_struct._idex = index;
}
long* Session::SessionReleaseIOCount::GetReleaseFlagPtr()
{
	return &_struct.releaseFlag;
}
long* Session::SessionReleaseIOCount::GetIoCountPtr()
{
	return &_struct.ioCount;
}

SessionManager::SessionManager(int sessionCount)
{
	_sessionMaxCount = sessionCount;
	_sessionList = new Session[sessionCount];
	_InitIndexStack();

	_sessionCount = 0;
	_sessionID = 1;
}

void SessionManager::_InitIndexStack()
{
	for (int i = 19999; i >= 0; i--)
	{
		_indexStack.Delete((unsigned short)i);
	}
}

Session::Session()
{
	_socket = 0;
	_ID._ulong64 = 0;
	_releaseIOFlag._all = 0;
	_releaseIOFlag._struct.releaseFlag = 1;
	
	_sendFlag = 0;

	ZeroMemory(&_clientAddr, sizeof(_clientAddr));
	ZeroMemory(&_sendOverLapped, sizeof(_sendOverLapped));
	ZeroMemory(&_recvOverLapped, sizeof(_recvOverLapped));

	sendData = 0;
	sendCount = 0;
	_recvBuffer = nullptr;

	_status = static_cast<long>(Session::Status::Active);
	
}

Session::~Session()
{

}

Session& SessionManager::operator[](int idex)
{
	return _sessionList[idex];
}

void SessionManager::Delete(unsigned short iDex)
{
	_sessionList[iDex].clear();
	_indexStack.Delete(iDex);
	InterlockedDecrement(&_sessionCount);
}

void Session::clear()
{
	_socket = 0;
	_ID._ulong64 = 0;

	ZeroMemory(&_clientAddr, sizeof(_clientAddr));

	_sendBuffer.Clear();
	_recvBuffer->DecrementUseCount();
	_recvBuffer = nullptr;

	InterlockedExchange(&_sendFlag, 0);
	sendData = 0;
	sendCount = 0;

	InterlockedExchange(&_status, static_cast<long>(Session::Status::Active));
}

void Session::init()
{
	InterlockedIncrement(_releaseIOFlag.GetIoCountPtr());
	InterlockedExchange(&_releaseIOFlag._struct.releaseFlag, 0);
	InterlockedExchange(&_sendFlag, 0);


	_recvBuffer = CPacket::Alloc();


}

bool SessionManager::_makeNewSession(unsigned short* outIDex, SOCKET* newSocket, SOCKADDR_IN* clientAddr)
{
	unsigned short iDex = _indexStack.Alloc();
	ULONG64 localID = _sessionID++;
	*outIDex = iDex;
	
	_sessionList[iDex].init();
	_sessionList[iDex]._ID.SetID(localID);
	_sessionList[iDex]._ID.SetIndex(iDex);
	_sessionList[iDex]._socket = *newSocket;
	_sessionList[iDex]._clientAddr = *clientAddr;



	g_SessionTotalCreateCount++; //acceptThread에서만 증가 연산 중
	InterlockedIncrement(&_sessionCount);
	InterlockedIncrement(&g_LoginSessionCount);

	return true;
}

Session& SessionManager::GetSession(int idex)
{
	return _sessionList[idex];
}



void Session::SwapRecvBuffer()
{
	//리시브 버퍼 스왑//
	CPacket* newBuf = CPacket::Alloc();
	CPacket* oldRecvbuf = _recvBuffer;
	newBuf->PutData(_recvBuffer->GetDataPtr(), _recvBuffer->GetDataSize());
	_recvBuffer = newBuf;
	if (oldRecvbuf->_usageCount != 1)
	{
		__debugbreak();
	}
	oldRecvbuf->DecrementUseCount();
}

