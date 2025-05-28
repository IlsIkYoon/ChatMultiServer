#pragma once
#include "resource.h"
#include "Player/DummySession.h"


//-----------------------------------------
// 더미클라이언트의 쓰레드들이 할 일
//-----------------------------------------
unsigned int DummyClientThreadFunc(void*);
//--------------------------------
// 실제 컨텐츠 로직 돌아가는 함수.
// 쓰레드에선 자기 세션만 로직을 돌림
//--------------------------------
bool ContentsWork(DummySession* mySessionArr, DWORD mySessionCount, DWORD fixedDeltaTime);
//--------------------------------
// 액션에 대해 끝냈다는 카운트를 올리고 내가 마지막이면 return true;
//--------------------------------
bool ThreadActionComplete();
//--------------------------------
// MoveStart패킷 만들어서 sendBuf에 인큐
//--------------------------------
bool SendMoveStartMessage(DummySession* _session);
//--------------------------------
// MoveStop패킷 만들어서 sendBuf에 인큐
//--------------------------------
bool SendMoveStopMessage(DummySession* _session);
//--------------------------------
// 세션 상태 보고 활성화 되어 있으면 하트비트 보내주기
//--------------------------------
bool SendHeartBeatMessage(DummySession* _session);
//--------------------------------
// LocalChatMessage를 SendBuf에 Enque
//--------------------------------
bool SendLocalChatMessage(DummySession* _session, char* chatMessage, int msgLen);
//--------------------------------
// 모든 세션 돌면서 Recv데이터 처리해주는 함수
//--------------------------------
bool HandleRecvData_All(DummySession* mySessionArr, DWORD dwMySessionCount);
//--------------------------------
// 세션의 recv데이터를 처리해주는 함수
//--------------------------------
bool HandleRecvData_Session(DummySession* _session);
//--------------------------------
// 네트워크 헤더를 뜯어서 직렬화버퍼에 담아서 내보내주는 함수
//--------------------------------
bool Decode_NetworkHeader(DummySession* _session, CPacket* ContentsPacket);
//--------------------------------
// 컨텐츠 페이로드를 가지고 실제 로직 실행
//--------------------------------
bool Decode_ContentsPacket(DummySession* _session, CPacket* ContentsPacket);
//--------------------------------
// 캐릭터 생성 메세지에 대한 처리 함수
//--------------------------------
bool CreateCharacter(DummySession* _session, CPacket* ContentsPacket);
//--------------------------------
// LocalChat메세지 핸들링 함수
//--------------------------------
bool HandleLocalChatMsg(DummySession* _session, CPacket* ContentsPacket);
//--------------------------------
// MoveStopComplete핸들링 함수
//--------------------------------
bool HandleMoveStopCompleteMsg(DummySession* _session, CPacket* ContentsPacket);
//--------------------------------
// ChatComplete 핸들링 함수
//--------------------------------
bool HandleChatCompleteMsg(DummySession* _session, CPacket* ContentsPacket);
//--------------------------------
// 메세지를 받아야 하는 세션의 StringCount를 올려주는 함수
//--------------------------------
bool CheckReceiversInSector(DummySession* _session, char* chatMsg, int msgLen);
//--------------------------------
// 내 ActionComplete카운트를 체크
//--------------------------------
bool CheckMyActionComplete();
//--------------------------------
// ChatEnd메세지를 보내줌
//--------------------------------
bool SendChatEndMessage(DummySession* _session);
//--------------------------------
// 네트워크 헤더를 다시 넣어줌
//--------------------------------
bool RestoreNetworkHeader(ClientHeader header, DummySession* _session);