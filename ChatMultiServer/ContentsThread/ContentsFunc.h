#pragma once


#include "ContentsResource.h"
#include "Player/Player.h"
#include "Msg/ContentsPacket.h"


class CLanServer : public NetWorkManager
{

public:
	CLanServer();

	void _OnMessage(CPacket* message, ULONG64 sessionID) override final;
	void _OnAccept(ULONG64 sessionID) override final;
	void _OnDisConnect(ULONG64 sessionID) override final;
	void _OnSend(ULONG64 sessionID) override final;

};




//------------------------------------
// printList의 size가 0이 될 때 까지 꺼내서 출력
//------------------------------------
void PrintString();


//----------------------------------------------
// todo// 쓰레드 종료에 대한 로직 구현해야함
//----------------------------------------------
void ShutDownAllThread();


//----------------------------------------------
// 컨텐츠에서 사용하는 리소스 초기화
// 컨텐츠 배열 메모리 할당 + 스택에 인덱스 값들 push
//----------------------------------------------
bool InitContentsResource();

//----------------------------------------------
// 플레이어 전부 돌면서 하트비트 시간 체크하는 함수
//----------------------------------------------
void TimeOutCheck();