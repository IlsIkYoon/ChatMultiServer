#pragma once
#include "Contents/ContentsResource.h"

extern unsigned long long g_PlayerID;
extern std::stack<int> g_playerIndexStack;



class Player;

extern Player* PlayerArr;

class Player {
public:

	enum class STATUS
	{
		DELETED = 0, WAIT_CREATE, ALIVE
	};

	DWORD _timeOut;
	BYTE _status;


private:
	bool _move;
	short _x;
	short _y;
	BYTE _direction;
	unsigned long long _ID;




public:

	Player()
	{
		_move = false;
		_status = static_cast<BYTE>(STATUS::DELETED);
		_direction = 0;
		_x = 0;
		_y = 0;
		_ID = 0;
		_timeOut = 0;
	}
	
	//------------------------------------------------------
	// 무브 플래그 켜고클라이언트에서 온 좌표 방향 그대로 기입
	//------------------------------------------------------
	bool MoveStart(BYTE Direction, int x, int y);

	bool Move(DWORD deltaTime);

	//------------------------------------------------------
	// 무브 플래그 끄고 클라이언트에서온 방향 좌표 그대로 기입
	//------------------------------------------------------
	void MoveStop(BYTE Direction, int x, int y);


	//------------------------------------------------------
	// 정적 배열이라 삭제에 준하는 함수
	//------------------------------------------------------
	void Clear();

	//------------------------------------------------------
	// 정적 배열이라 생성자에 준하는 함수
	//------------------------------------------------------
	void Init() = delete;
	void Init(ULONG64 sessionID);

	//------------------------------------------------------
	// Player가 유효한지를 리턴
	//------------------------------------------------------
	bool isAlive();

	unsigned long long GetID();
	void SetID(ULONG64 id)
	{
		_ID = id;
	}
	inline short GetX()
	{
		return _x;
	}
	inline short GetY()
	{
		return _y;
	}

};


//-----------------------------------------------
// 대기열 큐에 id를 넣어주는 함수
// 플레이어의 delete Flag도 true로 바꿔줌
//-----------------------------------------------
void EnqueueWaitingPlayerQ(ULONG64 id);
//-----------------------------------------------
// 대기열 큐에서 빼서 생성해주는 함수
//-----------------------------------------------
bool DequeueWaitingPlayerQ();