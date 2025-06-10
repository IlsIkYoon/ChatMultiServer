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
// 메세지 핸들링 함수들
//------------------------------------------------------
bool HandleLoginMessage(CPacket* message, ULONG64 sessionID);
bool HandleSectorMoveMessage(CPacket* message, ULONG64 sessionID);
bool HandleChatMessage(CPacket* message, ULONG64 sessionID);


//------------------------------------------------------
// dest에게 MoveStop을 완료했다는 메세지를 보내주는 함수
//------------------------------------------------------
void SendMoveStopCompleteMessage(ULONG64 destID);

//------------------------------------------------------
// 문자열에 NULL문자가 포함되어 있는지 체크해주는 함수
//------------------------------------------------------
bool CheckNullChar(WCHAR* message, DWORD len);


