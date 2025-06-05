#include "ContentsResource.h"
#include "Message.h"
#include "Sector/Sector.h"
#include "ContentsPacket.h"
#include "Player/Player.h"
#include "ContentsThread/ContentsFunc.h"
#include "CommonProtocol.h"
#include "ContentsThread/ContentsThreadManager.h"
//-----------------------------------------
// 출력을 위한 메세지 카운팅
//-----------------------------------------
unsigned long long g_loginResMsgCnt;
unsigned long long g_sectorMoveResMsgCnt;
unsigned long long g_chatResMsgCnt;
unsigned long long g_mychatResMsgCnt;


extern std::stack<int> g_playerIndexStack;
extern long long g_playerCount;
extern unsigned long long g_PlayerLogOut;

extern std::list<Player*> Sector[SECTOR_MAX][SECTOR_MAX];
extern std::recursive_mutex SectorLock[SECTOR_MAX][SECTOR_MAX];
extern int sectorXRange;
extern int sectorYRange;

extern unsigned long long g_PlayerLogInCount;
extern unsigned long long g_TotalPlayerCreate;
extern std::queue<ULONG64> g_WaitingPlayerAcceptQ;

extern CLanServer* ntServer;
extern CContentsThreadManager contentsManager;


void MsgSectorBroadCasting(void (*Func)(ULONG64 srcID, ULONG64 destID, CPacket* Packet), Player* _src, CPacket* Packet, bool SendMe)
{
	Player* pSrc = _src;

	typename std::list<Player*>::iterator pit; // = Sector[0][0].begin();
	int SectorX;
	int SectorY;

	SectorX = pSrc->sectorX;
	SectorY = pSrc->sectorY;

	//섹터 락을 전부 잡고 나서 시작
	{


		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (SectorX + i < 0 || SectorX + i >= sectorXRange) continue;
				if (SectorY + j < 0 || SectorY + j >= sectorYRange) continue;

				
				SectorLock[SectorX + i][SectorY + j].lock();
			}
		}

		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (SectorX + i < 0 || SectorX + i >= sectorXRange) continue;
				if (SectorY + j < 0 || SectorY + j >= sectorYRange) continue;

				for (pit = Sector[SectorX + i][SectorY + j].begin(); pit != Sector[SectorX + i][SectorY + j].end(); pit++)
				{
					if ((*pit)->_sessionID == pSrc->_sessionID && SendMe == false)
					{
						__debugbreak();
						continue;
					}
					if ((*pit)->isAlive() == false)
					{
						__debugbreak();
						continue;
					}
					if ((*pit)->_status < static_cast<BYTE>(Player::STATUS::PLAYER))
					{
						__debugbreak();
					}

					Func(pSrc->_sessionID, (*pit)->_sessionID, Packet);

				}
				/*
				for (auto* it : Sector[SectorX + i][SectorY + j])
				{
					if (it->GetID() == pSrc->GetID() && SendMe == false)
					{
						continue;
					}
					if (it->isAlive() == false) continue;

					Func(pSrc->GetID(), it->GetID(), Packet);

				}
				*/
			}
		}


		for (int i = -1; i < 2; i++)
		{
			for (int j = -1; j < 2; j++)
			{
				if (SectorX + i < 0 || SectorX + i >= sectorXRange) continue;
				if (SectorY + j < 0 || SectorY + j >= sectorYRange) continue;


				SectorLock[SectorX + i][SectorY + j].unlock();
			}
		}
	}

}

void ContentsSendPacket(ULONG64 srcID, ULONG64 destID, CPacket* packet)
{
	if (srcID == destID)
	{
		InterlockedIncrement(&g_mychatResMsgCnt);
	}
	ntServer->SendPacket(destID, packet);

}

void SendChatResPacket(ULONG64 srcID, ULONG64 destID, CPacket* packet) 
{
	CPacket* sendMsg;
	unsigned short playerIndex;
	Player* localPlayerList;

	if (srcID == destID)
	{
		InterlockedIncrement(&g_mychatResMsgCnt);
	}

	//-----------------------------
	// payLoad 중 단독 데이터
	//-----------------------------
	WORD Type;
	INT64 AccountNo;

	playerIndex = NetWorkManager::GetIndex(destID);
	localPlayerList = contentsManager.playerList->playerArr;

	Type = en_PACKET_CS_CHAT_RES_MESSAGE;
	AccountNo = localPlayerList[playerIndex].accountNo;

	sendMsg = CPacket::Alloc();
	*sendMsg << Type;
	//*sendMsg << AccountNo;
	sendMsg->PutData(packet->GetDataPtr(), packet->GetDataSize());

	ntServer->SendPacket(destID, sendMsg);
	sendMsg->DecrementUseCount();
	InterlockedIncrement(&g_chatResMsgCnt);
}

void SendLoginResPacket(ULONG64 sessionID)
{
	int playerIndex;
	Player* localPlayerList;
	CPacket* sendMsg;

	WORD Type;
	BYTE Status;	//0 : 실패 1: 성공
	INT64 AccountNo;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = ntServer->GetIndex(sessionID);

	Type = en_PACKET_CS_CHAT_RES_LOGIN;
	Status = 1;
	AccountNo = localPlayerList[playerIndex].accountNo;

	sendMsg = CPacket::Alloc();
	*sendMsg << Type;
	*sendMsg << Status;
	*sendMsg << AccountNo;

	ntServer->SendPacket(sessionID, sendMsg);

	sendMsg->DecrementUseCount();

	InterlockedIncrement(&g_PlayerLogInCount);
	InterlockedIncrement(&g_TotalPlayerCreate);
	InterlockedIncrement(&g_loginResMsgCnt);

}


bool HandleLoginMessage(CPacket* message, ULONG64 sessionID)
{
	//char tokenRetval;
	DWORD playerIndex;
	Player* localPlayerList;

	//------------------------------------
	// 메세지 페이로드
	//------------------------------------
	INT64 AccountNo; //캐릭터 키
	WCHAR ID[20]; //null포함
	WCHAR Nickname[20]; //null포함
	char SessionKey[64]; //인증 토큰
	
	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = NetWorkManager::GetIndex(sessionID);
	//------------------------------------
	// 오류 체크
	//------------------------------------
	if (localPlayerList[playerIndex]._status != static_cast<BYTE>(Player::STATUS::SESSION))
	{
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	//------------------------------------


	*message >> AccountNo;
	message->PopFrontData(sizeof(WCHAR) * 20, (char*)ID);
	message->PopFrontData(sizeof(WCHAR) * 20, (char*)Nickname);
	message->PopFrontData(sizeof(char) * 64, SessionKey);

	//tokenRetval = CheckLoginToken(AccountNo, SessionKey); -> 로그인 서버에서 보내준 토큰 자료구조를 검색해서 키값에 맞는 토큰인지 확인
	//이후 tokenRetval에 따른 로직분기 

	//LoadCharacterDataOnLogin(AccountNo, userId); 
	//토큰이 유효하다면 DB에서 캐릭터 데이터 긁어오기 요청 후 긁어온 뒤에 결과에 따라 Login_res 메세지 전송

	localPlayerList[playerIndex].Init(sessionID);
	localPlayerList[playerIndex].accountNo = AccountNo;
	wcsncpy_s(localPlayerList[playerIndex].ID, ID, 20);
	wcsncpy_s(localPlayerList[playerIndex].nickname, Nickname, 20);

	SendLoginResPacket(sessionID);

	return true;
}


bool HandleSectorMoveMessage(CPacket* message, ULONG64 sessionID)
{
	//플레이어의 섹터 정보를 변경 시켜주고 
	//섹터 이동에 따른 섹터 처리를 함
	Player* localPlayerList;
	DWORD playerIndex;
	WORD oldSectorX;
	WORD oldSectorY;
	//-------------------------------
	// 메세지 페이로드
	//-------------------------------
	INT64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = NetWorkManager::GetIndex(sessionID);

	*message >> AccountNo;
	*message >> SectorX;
	*message >> SectorY;


	//-----------------------------------------
	// 오류 체크
	//-----------------------------------------
	if (localPlayerList[playerIndex]._status < static_cast<BYTE>(Player::STATUS::PENDING_SECTOR))
	{
		__debugbreak();
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	if (SectorX > SECTOR_MAX || SectorY > SECTOR_MAX)
	{
		__debugbreak();
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	if (AccountNo != localPlayerList[playerIndex].accountNo)
	{
		__debugbreak();
		ntServer->DisconnectSession(sessionID);
		return false;
	}
	//-----------------------------------------

	if (localPlayerList[playerIndex]._status == static_cast<BYTE>(Player::STATUS::PENDING_SECTOR))
	{
		//섹터 첫 배치 작업
		localPlayerList[playerIndex]._status = static_cast<BYTE>(Player::STATUS::PLAYER); //섹터 배치로 플레이어 승격
		localPlayerList[playerIndex].sectorX = SectorX;
		localPlayerList[playerIndex].sectorY = SectorY;

		{
			std::lock_guard guard(SectorLock[SectorX][SectorY]);
			Sector[SectorX][SectorY].push_back(&localPlayerList[playerIndex]);
		}

	}
	else 
	{
		//섹터 옮겨주기 작업

		oldSectorX = localPlayerList[playerIndex].sectorX;
		oldSectorY = localPlayerList[playerIndex].sectorY;

		localPlayerList[playerIndex].sectorX = SectorX;
		localPlayerList[playerIndex].sectorY = SectorY;

		SyncSector(sessionID, oldSectorX, oldSectorY);

	}
	SendSectorMoveResPacket(sessionID);


	return true;
}

bool HandleChatMessage(CPacket* message, ULONG64 sessionID)
{
	DWORD playerIndex;

	INT64 AccountNo;
	WORD MessageLen;
	WCHAR* Message; //null 미포함 // messageLen / 2길이
	Player* localPlayerList;

	localPlayerList = contentsManager.playerList->playerArr;

	*message >> AccountNo;
	*message >> MessageLen;

	Message = new WCHAR[MessageLen / 2];

	if (message->GetDataSize() != MessageLen)
	{
		__debugbreak();
	}
	message->PopFrontData(MessageLen, (char*)Message);

	playerIndex = NetWorkManager::GetIndex(sessionID);


	//Res메세지 페이로드
	WORD Type;
	//INT64 AccountNo;
	WCHAR ID[20] = { 0 };
	WCHAR Nickname[20] = { 0 };
	//WORD MessageLen;
	//WCHAR* Message;

	Type = en_PACKET_CS_CHAT_RES_MESSAGE;
	wcsncpy_s(ID, localPlayerList[playerIndex].ID, 20);
	wcsncpy_s(Nickname, localPlayerList[playerIndex].nickname, 20);

	CPacket* sendMsg; //공용 데이터용 보관 장소
	sendMsg = CPacket::Alloc();

	*sendMsg << Type;
	*sendMsg << AccountNo;
	sendMsg->PutData((char*)ID, sizeof(WCHAR) * 20);
	sendMsg->PutData((char*)Nickname, sizeof(WCHAR) * 20);
	*sendMsg << MessageLen;
	sendMsg->PutData((char*)Message, MessageLen);


	MsgSectorBroadCasting(ContentsSendPacket, &localPlayerList[playerIndex], sendMsg, true);

	sendMsg->DecrementUseCount();

	delete[] Message;

	return true;
}


void SendSectorMoveResPacket(ULONG64 sessionID)
{
	int playerIndex;
	Player* localPlayerList;
	CPacket* sendMsg;

	WORD Type;
	INT64 AccountNo;
	WORD SectorX;
	WORD SectorY;

	localPlayerList = contentsManager.playerList->playerArr;
	playerIndex = NetWorkManager::GetIndex(sessionID);

	Type = en_PACKET_CS_CHAT_RES_SECTOR_MOVE;
	AccountNo = localPlayerList[playerIndex].accountNo;
	SectorX = localPlayerList[playerIndex].sectorX;
	SectorY = localPlayerList[playerIndex].sectorY;

	sendMsg = CPacket::Alloc();
	*sendMsg << Type;
	*sendMsg << AccountNo;
	*sendMsg << SectorX;
	*sendMsg << SectorY;


	ntServer->SendPacket(sessionID, sendMsg);

	sendMsg->DecrementUseCount();
	InterlockedIncrement(&g_sectorMoveResMsgCnt);

}