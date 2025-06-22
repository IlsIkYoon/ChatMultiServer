#pragma once
#include <Windows.h>

#define Protocol_Code 0x89
#define Fixed_Code 0xa9
#define Random_Key 0x31

#pragma pack(push, 1)
//struct NetWorkHeader
//{
//	WORD len;
//};
struct NetWorkHeader
{
	BYTE code;
	WORD len;
	BYTE randkey;
	BYTE checkSum;
};
struct ContentHeader
{
	//BYTE len;
	WORD type;
};
#pragma pack(pop)

#define stPacket_Chat_Client_CreateCharacter	0
//
//	8	-	CharacterKey
//	4	-	x 좌표
//	4	-	y 좌표
//

#define stPacket_Client_Chat_MoveStart		1
//
//	1	-	Direction
//	4	-	x 좌표
//	4	-	y 좌표
//

#define stPacket_Client_Chat_MoveStop		2
//
//	1	-	Direction
//	4	-	x 좌표
//	4	-	y 좌표
//

#define stPacket_Client_Chat_LocalChat		3
//
//	1	-	ChatMessageLen	
//	ChatMessageLen - ChatMessage
//

#define stPacket_Chat_Client_LocalChat		4
//
//	8	-	CharacterKey
//	1	-	NickNameLen	// 없음
//	NickNameLen - NickName	//없음
//	1	-	ChatMessageLen
//	ChatMessageLen - ChatMessage
//

#define stPacket_Client_Chat_HeartBeat		5
//
//	데이터 없음
//

#define stPacket_Chat_Client_MoveStopOk		6


#define stPacket_Client_Chat_ChatEnd	7

#define stPacket_Chat_Client_ChatComplete	8


//-----------------------------------------------------------------
// 화면 이동 범위.
//-----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400



//-----------------------------------------------------------------
// 캐릭터 이동 속도   // 25fps 기준 이동속도
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X	6	
#define dfSPEED_PLAYER_Y	4	

//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE		50


#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7
