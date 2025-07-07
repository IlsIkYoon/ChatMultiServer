#pragma once
#include "ContentsResource.h"
#include "Player/Player.h"
#include "Msg/ContentsPacket.h"


class CWanServer : public CWanManager
{

public:
	CWanServer();

	void _OnMessage(CPacket* message, ULONG64 sessionID) override final;
	void _OnAccept(ULONG64 sessionID) override final;
	void _OnDisConnect(ULONG64 sessionID) override final;
	void _OnSend(ULONG64 sessionID) override final;

};

//----------------------------------------------
// todo// 쓰레드 종료에 대한 로직 구현해야함
//----------------------------------------------
void ShutDownAllThread();

//----------------------------------------------
// 플레이어 전부 돌면서 하트비트 시간 체크하는 함수
//----------------------------------------------
void TimeOutCheck();

bool UpdateMonitorData();