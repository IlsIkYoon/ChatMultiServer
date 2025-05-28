#pragma once
#include "Player/DummySession.h"


class Action
{
public:
	enum class Status
	{
		ACTION_CONNECT =  0,
		ACTION_SENDCHATMSG,
		ACTION_CHECKCHATMSG,
		ACTION_MOVE,
		ACTION_MOVESTOP,
		ACTION_CHECKMOVESTOPMSG,
		ACTION_CHECKDISCONNECT
	
	};




};



//----------------------------
// Connect액션 때 처리될 컨텐츠 로직
//----------------------------
bool HandleConnectAction(DummySession* mySessionArr, DWORD mySessionCount);
//----------------------------
// Session에 대한 blocking Connect 실행
//----------------------------
bool DoSessionConnect(DummySession* session);
//----------------------------
// Move액션 때 처리될 컨텐츠 로직
// MoveStart패킷을 보내면서 Move 상태로 바꿔주고 
// MoveStop패킷을 보내면서 상태를 MoveStop완료 패킷을 기다리는 상태로 변경
//----------------------------
bool HandleMoveAction(DummySession* mySessionArr, DWORD mySessionCount, DWORD fixedDeltaTime);
//----------------------------
// Msg가 다 왔는지 체크하는 상태. 모든 세션의 메세지가 다 왔는지를 체크
//----------------------------
bool HandleCheckMsgAction(DummySession* mySessionArr, DWORD mySessionCount);
//----------------------------
// 채팅 메세지를 무작위로 보내고 세션의 자료구조에 넣어주는 함수
//----------------------------
bool HandleSendChatMsgAction(DummySession* mySessionArr, DWORD mySessionCount);
//----------------------------
// MoveStop메세지가 모두 올 때 까지 기다리는 함수
//----------------------------
bool HandleCheckMoveStopAction(DummySession* mySessionArr, DWORD mySessionCount);
//----------------------------
// DisConnect되는 걸 기다리고 DisConnect시키는 함수
//----------------------------
bool HandleCheckDisconnectAction(DummySession* mySessionArr, DWORD mySessionCount);
//----------------------------
// MoveStop패킷 날리고 응답 기다리는 함수
//----------------------------
bool HandleMoveStopAction(DummySession* mySessionArr, DWORD mySessionCount);

//----------------------------
// Action을 변경해주고 g_ActionComplete를 초기화해줌
//----------------------------
bool ChangeAction(char action);

