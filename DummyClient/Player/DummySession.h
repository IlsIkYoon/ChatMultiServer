#pragma once

#include "resource.h"


class DummySession : public Session
{
public:
	enum class DummyStatus
	{
        Idle = 0,            // 아무것도 안 하고 있는 초기 상태 생성되지 않은 걸로 침
        Created,
        Disconnected,
        Connecting,          // 서버에 연결 시도 중
        Connected,           // 연결 완료, 초기 통신 준비
        ReadyToMoveStart, //MoveStart 패킷 보낼 준비
        SendingMove,         // 이동 명령 전송 중
        WaitingMoveEnd,      // 이동 응답 기다리는 중
        RecvdMoveEnd,
        SendingChat,         // 채팅 전송 중
        WaitingChatEnd,      // 채팅 응답 기다리는 중
        RecvdChatComplete,
        Disconnecting,       // 종료 시도 중
        WaitingDisconnect   // 서버가 끊길 기다리는 중
	};

    int _x;
    int _y;
    char _direction;
    bool _created;
    
    std::mutex _sessionLock;

    bool _move;

	long dummyStatus;
    RingBuffer _recvQ{ 4 * 1024 };
    RingBuffer _sendQ{ 4 * 1024 };


    std::map<std::string, long> stringCounter;
    bool allMsgRecvd;
    
    //------------------------------------
    // 생성자에서 Status를 Idle로 초기화 중
    //------------------------------------
    DummySession();

    void Dummy_Clear();
    bool Move(DWORD deltaTime);
    //------------------------------------
    // SendBuffer에 enque를 하고 enque가 실패하면 debugbreak();
    //------------------------------------
    bool SendBufferEnque(char* buffer, int size, unsigned int* enqueResult);
    //------------------------------------
    // RecvBuffer에 enque를 하고 enque가 실패하면 debugbreak();
    //------------------------------------
    bool RecvBufferEnque(char* buffer, int size, unsigned int* enqueResult);
    //------------------------------------
    // SendBuffer Deque 에러 예외처리 래핑 함수
    //------------------------------------
    bool SendBufferDeque(char* buffer, int size, unsigned int* dequeResult);
    //------------------------------------
    // RecvBuffer Deque 에러 예외처리 래핑 함수
    //------------------------------------
    bool RecvBufferDeque(char* buffer, int size, unsigned int* dequeResult);



};


//-------------------------------------------------------
// 섹터 변경 시에 섹터 이동해주는 함수
//-------------------------------------------------------
bool SyncSector(DummySession* session, int beforeX, int beforeY);