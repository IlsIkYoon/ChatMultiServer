#pragma once

#pragma comment(lib, "NetWorkLib.lib")

#include "NetWorkLib_Unified.h"


#define SESSION_PER_THREAD 640

#define NetWorkProtocol 0xa9

#define SECTOR_RATIO 100 //100분의 1 비율 6400에서 나눠서 떨어지는 수를 하는게 좋다
#define FrameRate 25
#define FrameSec (1000 / FrameRate)
#define FrameError 20

#define TRUST_MODE 0

//----------------------------------------------------------------
//# 화면 이동영역-------------------------- -
//----------------------------------------------------------------
#define dfRANGE_MOVE_TOP	0
#define dfRANGE_MOVE_LEFT	0
#define dfRANGE_MOVE_RIGHT	6400
#define dfRANGE_MOVE_BOTTOM	6400

#define MAX_SECTOR_X dfRANGE_MOVE_RIGHT/SECTOR_RATIO
#define MAX_SECTOR_Y dfRANGE_MOVE_BOTTOM/SECTOR_RATIO



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



//---------------------------------
// Session Status
//---------------------------------
#define NotAlive 0
#define ReadyToAction 1
#define ProcessInAction 2
#define ReadyToComplete 3
#define ActionComplete 4





#define STRINGCOUNT 20
#define SEND_PROBABILITY 5


#define THREAD_ACTION_INCOMPLETE 0
#define THREAD_ACTION_COMPLETE 1
