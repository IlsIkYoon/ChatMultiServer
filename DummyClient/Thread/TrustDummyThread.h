#pragma once
#include "Player/DummySession.h"


//--------------------------------------
// 메세지 체크를 하지 않는 더미 로직
//--------------------------------------
unsigned int DummyTrustThreadFunc(void*);

//--------------------------------------
// Trust용 컨텐츠 work(Action 개념이 없음)
//--------------------------------------
bool Trust_ContentsWork(DummySession* sessionArr, DWORD mySessionCount, DWORD fixedDeltaTime);
//--------------------------------------
// 확률에 따라 movestart 혹은 chat 혹은 아무것도 하지 않는 함수
//--------------------------------------
bool DoChatOrMoveStart(DummySession* _session);
//--------------------------------------
// 랜덤 채팅 메세지를 뽑아서 보내주는 함수
//--------------------------------------
bool SendRandomChat(DummySession* _session);