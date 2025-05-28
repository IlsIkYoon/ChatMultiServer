#pragma once
#include "resource.h"
#include "Player/DummySession.h"

class CThreadNetworkManager
{
public:
	fd_set* myFdReadSet;
	fd_set* myFdWriteSet;
	DummySession* mySessionArr;
	DWORD fdSetCount;

	DWORD dwMyIndex;
	bool bLastThread;
	DWORD dwMySessionCount;

	//-----------------------------------------
	// 생성자
	//-----------------------------------------
	CThreadNetworkManager();
	//-----------------------------------------
	// 내 인덱스 번호와 세션 카운트를 세팅해줌
	// 내가 마지막 쓰레드인지를 리턴 (true면 마지막 쓰레드)
	//-----------------------------------------
	bool SetMySessionCount();
	//-----------------------------------------
	// fdSet의 배열 개수를 설정해주는 함수. 내부에서 동적 할당
	//-----------------------------------------
	bool SetMyFDSET();
	//-----------------------------------------
	// 컨텐츠 쓰레드 메인 루프에서 소켓 네트워크 처리를 하는 함수
	//-----------------------------------------
	bool HandleNetworkEvent();
	//-----------------------------------------
	// FDSet을 제로로 밀어줌
	//-----------------------------------------
	bool ZeroFdSet();
	//-----------------------------------------
	// FDSet 등록
	//-----------------------------------------
	bool RegistFdSet();
	//-----------------------------------------
	// Select함수 실행
	//-----------------------------------------
	bool DoSelectFunc();

	//-----------------------------------------
	// Select ReadSet일때 처리하는 함수
	//-----------------------------------------
	bool HandleReadSelect(DummySession* _session);
	//-----------------------------------------
	// Select WriteSet일때 처리하는 함수
	//-----------------------------------------
	bool HandleWriteSelect(DummySession* _session);
	//------------------------------------------
	// ID에서 Index만 추출해주는 함수 (현재 사용하지 않음)
	//------------------------------------------
	static unsigned short GetIndex(ULONG64 target);
	//------------------------------------------
	// 링거 걸어주는 옵션. 논블러킹 소켓은 셀렉트 모델이라 안 함
	//------------------------------------------
	static bool SetSocketOption(SOCKET* socket);
};