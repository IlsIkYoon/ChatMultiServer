#pragma once
#include <WinSock2.h>
#include "RingBuffer.h"
#include "PacketBuffer.h"
#include <vector>
#include <string>
#include "Protocol.h"
extern LONG moveStopCnt;
extern LONG chatCompleteCnt;
extern LONG chatPlayerCnt;
extern LONG totalMessageRecv;
extern LONG disconnectFail;
extern LONG wrongChatCnt;
extern LONG forDebug;
extern LONG packetTypeErrorCnt;
extern LONG packetlenErrorCnt;
extern LONG fixedCodeErrorCnt;
extern LONG checkSumErrorCnt;
extern LONG connectFail;
extern LONG totalMoveStopRecv;
extern LONG MoveStopOverlapError;
extern LONG connectTryCnt;
extern LONG charOverlappedCnt;

extern int trustMode;
//extern LONG connectCnt;
extern const int MSG_COUNT;

extern std::vector<std::string> g_randomMessages;

//#pragma pack(push, 1)
//
//struct NetWorkHeader
//{
//	WORD len;
//};
//
//struct ContentHeader
//{
//	BYTE len;
//	WORD type;
//};
//
//#pragma pack(pop)

struct st_SECTOR_POS
{
	int iX;
	int iY;
};

struct Session
{
	SOCKET socket;
	RingBuffer recvQ;
	RingBuffer sendQ;
	bool isCharacterActive;
	bool isSession;
	bool isConnected;
	bool isReadyForChat;
	bool checkHeartBeat;
	DWORD rtt;

	unsigned long long id;
	int shX;
	int shY;
	BYTE byDirection;
	bool isMoving;
	bool moveStopRecv;
	bool chatSent;
	st_SECTOR_POS CurSector;
	st_SECTOR_POS OldSector;

	std::vector<LONG> messageCount;

	Session()
		:socket(INVALID_SOCKET), recvQ(1000), sendQ(1000), isSession(false), isCharacterActive(false), isConnected(false), chatSent(false), isReadyForChat(false),
		id(0), shX(0), shY(0), byDirection(0), isMoving(false), CurSector{ 0,0 }, OldSector{ 0,0 }, moveStopRecv(false), checkHeartBeat(false), rtt(0)
	{
		messageCount.resize(MSG_COUNT);
	}

};

bool connectSession(Session* sess, const char* serverIP, int serverPort);

bool netProc_Recv(Session* session);

bool RecvProcess(Session* session, PacketBuffer* packet);

bool netProc_Send(Session* session);

/////////////////////////////

bool Packet_CreateCharacter(Session* session, PacketBuffer* packet);

bool Packet_LocalChat(Session* session, PacketBuffer* packet);

bool Packet_MoveStopOk(Session* session, PacketBuffer* packet);

bool Packet_ChatComplete(Session* sesion, PacketBuffer* packet);

////////////////////////////

void SendPacket_Unicast(Session* session, PacketBuffer* packet);

//encode decode
void SetCheckSum(PacketBuffer* buffer);

int GetPayLoadCheckSum(PacketBuffer* buffer);

void Encode(PacketBuffer* buffer);

void Decode(PacketBuffer* buffer);