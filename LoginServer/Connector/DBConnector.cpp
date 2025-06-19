#include "DBConnector.h"
#include "CommonProtocol.h"


bool CDBManager::LoginDataRequest(CPacket* message, ULONG64 characterKey)
{
	//채워야 하는 데이터
	//		BYTE	Status				// 0 (세션오류) / 1 (성공) ...  하단 defines 사용
	//
	//		WCHAR	ID[20]				// 사용자 ID		. null 포함
	//		WCHAR	Nickname[20]		// 사용자 닉네임	. null 포함

	BYTE Status;
	WCHAR ID[20];
	WCHAR Nickname[20];

	//DBQuery날려서 결과 확인 리턴값에 따라 Status에 값 넣기
	//거기서 얻은 데이터 ID와 NickName에 넣어주기
	//todo//일단 디비 조회 없이 다 성공하는 로직으로 진행
	Status = en_PACKET_CS_LOGIN_RES_LOGIN::dfLOGIN_STATUS_OK;
	wcscpy_s(ID, L"ID_FIXED");
	wcscpy_s(Nickname, L"NICKNAME_FIXED");

	*message << Status;
	message->PutData((char*)ID, sizeof(WCHAR) * 20);
	message->PutData((char*)Nickname, sizeof(WCHAR) * 20);

	//동기로 작동되는 DB작업하기
	//패킷에 ID, Nickname넣어주기


	return true;
}