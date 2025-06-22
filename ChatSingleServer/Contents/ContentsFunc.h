#pragma once

#include "ContentsResource.h"
#include "Player/Player.h"
#include "ContentsPacket.h"


class CLanServer : public NetWorkManager
{

public:
	CLanServer();

	void _OnMessage(CPacket* message, ULONG64 ID) override final;
	void _OnAccept(ULONG64 ID) override final;
	void _OnDisConnect(ULONG64 ID) override final;
	void _OnSend(ULONG64 ID) override final;

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
// 컨텐츠 메세지 큐에서 꺼내서 스위치문으로 로직 처리하는 
// 쓰레드 함수
//----------------------------------------------
unsigned int ContentsThreadFunc(void*);

bool HandleContentJob();

//----------------------------------------------
// 컨텐츠에서 사용하는 리소스 초기화
// 컨텐츠 배열 메모리 할당 + 스택에 인덱스 값들 push
//----------------------------------------------
bool InitContentsResource();


//----------------------------------------------
// 매 프레임마다 해야할 일 
//----------------------------------------------
void UpdateContentsLogic(DWORD deltaTime);

//----------------------------------------------
// 플레이어 전부 돌면서 하트비트 시간 체크하는 함수
//----------------------------------------------
void TimeOutCheck();