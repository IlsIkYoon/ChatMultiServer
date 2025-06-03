#pragma once
#include "ContentsResource.h"

//todo//
//접속을 종료하면 그냥 바로 꺼내야 하는데 
//그러려면 단순한 큐가 아닌게 좋긴 하지 ??
//inasive Q 가자
//g_WaitingAccessPlayerQ
//그리고 얘는 출력 될 수 있게 ㄱㄱ

extern unsigned long long g_PlayerID;
extern std::stack<int> g_playerIndexStack;


class Player {
public:

	enum class STATUS
	{
		IDLE = 0, SESSION, PLAYER
	};

	DWORD _timeOut;
	BYTE _status;


public:
	bool _move;
	short sectorX;
	short sectorY;
	BYTE _direction;
	unsigned long long accountNo;
	WCHAR nickname[20];
	WCHAR ID[20];



public:

	Player()
	{
		_move = false;
		_status = static_cast<BYTE>(STATUS::IDLE);
		_direction = 0;
		sectorX = 0;
		sectorY = 0;
		accountNo = 0;
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
		accountNo = id;
	}
	inline short GetX()
	{
		return sectorX;
	}
	inline short GetY()
	{
		return sectorY;
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