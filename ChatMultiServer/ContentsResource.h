#pragma once

#include "NetWorkLib_Unified.h"

#define CONTENTS_THREADCOUNT 1
#define PLAYER_MAXCOUNT 100


#define NetWorkProtocol 0xa9

#define SECTOR_RATIO 100 //100분의 1 비율 6400에서 나눠서 떨어지는 수를 하는게 좋다
#define FrameRate 25
#define FrameSec (1000 / FrameRate)
#define FrameError 20

//----------------------------------------------------------------
//# 화면 이동영역-------------------------- -
//----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400


//-----------------------------------------------------------------
// 이동 오류체크 범위
//-----------------------------------------------------------------
#define dfERROR_RANGE		50 //나중에 수정


#define PLAYERMAXCOUNT 5000

#define LL 0x00
#define RR 0x04


#define dfERROR_RANGE		50

//// 30초 이상이 되도록 아무런 메시지 수신도 없는경우 접속 끊음.
#define dfNETWORK_PACKET_RECV_TIMEOUT	30000


//-----------------------------------------------------------------
// 캐릭터 이동 속도   // 25fps 기준 이동속도
//-----------------------------------------------------------------
#define dfSPEED_PLAYER_X	6	// 3   50fps
#define dfSPEED_PLAYER_Y	4	// 2   50fps



//-----------------------------------------------------------------
// 캐릭터 방향 
//-----------------------------------------------------------------
#define dfPACKET_MOVE_DIR_LL					0
#define dfPACKET_MOVE_DIR_LU					1
#define dfPACKET_MOVE_DIR_UU					2
#define dfPACKET_MOVE_DIR_RU					3
#define dfPACKET_MOVE_DIR_RR					4
#define dfPACKET_MOVE_DIR_RD					5
#define dfPACKET_MOVE_DIR_DD					6
#define dfPACKET_MOVE_DIR_LD					7




struct KeyTokenTimeNode
{
	unsigned long long characterKey;
	char Token[64];
	DWORD time;
};