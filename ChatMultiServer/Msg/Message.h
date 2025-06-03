#pragma once
#include "ContentsResource.h"
#include "Player/Player.h"


//-----------------------------------------------------
// sendpacket을 호출해주는 함수 (BroadCasting함수에 전달 예정)
//-----------------------------------------------------
void ContentsSendPacket(ULONG64 srcID, ULONG64 destID, CPacket* packet);

//-----------------------------------------------------
// 메세지 전송용 함수들
//-----------------------------------------------------
void SendLoginResPacket(ULONG64 sessionID);
void SendSectorMoveResPacket(ULONG64 sessionID);



//-----------------------------------------------------
// 주변 모두에게 함수를 호출해주는 함수 뿌려주는 함수
//-----------------------------------------------------
void MsgSectorBroadCasting(void (*Func)(ULONG64 srcID, ULONG64 destID, CPacket* packet), Player* _src, CPacket* Packet, bool SendMe);

//------------------------------------------------------
// MoveStart메세지를 받으면 그 아이디를 클라이언트 좌표로 동기화 후 이동 플래그를 세워줌
//------------------------------------------------------
bool HandleMoveStartMsg(CPacket* payLoad, ULONG64 sessionID);
//------------------------------------------------------
// MoveStop메세지에 있는 좌표로 동기화 후 이동 플래그 꺼줌. MoveStop메세지를 받았다는 확인 메세지 송신
//------------------------------------------------------
bool HandleMoveStopMsg(CPacket* payLoad, ULONG64 sessionID);
//------------------------------------------------------
//섹터 주변에 받은 메세지를 뿌려주는 함수
//------------------------------------------------------
bool HandleLocalChatMsg(CPacket* payLoad, ULONG64 sessionID);
//------------------------------------------------------
//플레이어의 하트비트 시간을 조정해주는 함수
//------------------------------------------------------
bool HandleHeartBeatMsg(ULONG64 sessionID);
//------------------------------------------------------
//모든 채팅 메세지의 송신이 완료 되었다는 의미의 채팅 메세지
//------------------------------------------------------
bool HandleChatEndMsg(ULONG64 sessionID);

//------------------------------------------------------
// Create 메세지를 받아서 실제로 생성해주는 함수
//------------------------------------------------------
bool HandleCreatePlayer(ULONG64 sessionID);
//------------------------------------------------------
// Delete 메세지를 받아서 delete작업을 해주는 함수
//------------------------------------------------------
bool HandleDeletePlayer(ULONG64 sessionID);


//------------------------------------------------------
// 메세지 핸들링 함수들
//------------------------------------------------------
bool HandleLoginMessage(CPacket* message, ULONG64 sessionID);
bool HandleSectorMoveMessage(CPacket* message, ULONG64 sessionID);
bool HandleChatMessage(CPacket* message, ULONG64 sessionID);


//------------------------------------------------------
// dest에게 MoveStop을 완료했다는 메세지를 보내주는 함수
//------------------------------------------------------
void SendMoveStopCompleteMessage(ULONG64 destID);


