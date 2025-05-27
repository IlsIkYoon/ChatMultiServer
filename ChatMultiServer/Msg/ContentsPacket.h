#pragma once
#include "ContentsResource.h"

struct stHeader
{
#pragma pack(push, 1)
	BYTE size;
	WORD type;
#pragma pack(pop)
};


#define stPacket_Login_Chat_Token	
//
//	8	-	CharaceterKey
//	4	-	Token
//

#define stPacket_Client_Chat_Token
//
//	8	-	CharaceterKey
//	4	-	Token
//

#define stPacket_Chat_Client_CreateCharacter	0
//
//	8	-	CharacterKey
//	4	-	x ÁÂÇ¥
//	4	-	y ÁÂÇ¥
//

#define stPacket_Client_Chat_MoveStart		1
//
//	1	-	Direction
//	4	-	x ÁÂÇ¥
//	4	-	y ÁÂÇ¥
//

#define stPacket_Client_Chat_MoveStop		2
//
//	1	-	Direction
//	4	-	x ÁÂÇ¥
//	4	-	y ÁÂÇ¥
//

#define stPacket_Client_Chat_LocalChat		3
//
//	1	-	ChatMessageLen	
//	ChatMessageLen - ChatMessage
//

#define stPacket_Chat_Client_LocalChat		4
//
//	8	-	CharacterKey
//	1	-	NickNameLen //»ý·«
//	NickNameLen - NickName //»ý·«
//	1	-	ChatMessageLen
//	ChatMessageLen - ChatMessage
//

#define stPacket_Client_Chat_WorldChat	
//
//	1	-	ChatMessageLen	
//	ChatMessageLen - ChatMessage
//

#define stPacket_Chat_Client_WorldChat	
//
//	1	-	NickNameLen
//	NickNameLen - NickName
//	1	-	ChatMessageLen
//	ChatMessageLen - ChatMessage
//

#define stPacket_Client_Chat_DirectChat	
//
//	1	-	TargetNickNameLen
//	TargetNickNameLen - TargetNickName
//	1	-	ChatMessageLen
//	ChatMessageLen - ChatMessage
//

#define stPacket_Chat_Client_DirectChat	
//
//	1	-	FromNickNameLen
//	FromNickNameLen - FromNickName
//	1	-	ChatMessageLen
//	ChatMessageLen - ChatMessage
//

#define stPacket_Client_Chat_HeartBeat		5
//
//	µ¥ÀÌÅÍ ¾øÀ½
//

#define stPacket_Chat_Client_MoveStopComplete		6


#define stPacket_Client_Chat_ChatEnd	7


#define stPacket_Chat_Client_ChatComplete	8


#define stJob_CreatePlayer 11
// µ¥ÀÌÅÍ ¾øÀ½

#define stJob_DeletePlayer 12
// µ¥ÀÌÅÍ ¾øÀ½


