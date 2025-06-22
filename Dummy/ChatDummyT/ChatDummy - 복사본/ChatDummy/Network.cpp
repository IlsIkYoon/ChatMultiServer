#include "Network.h"
#include "Protocol.h"
#include "Sector.h"
#include <stdio.h>
#include <WS2tcpip.h>
#include <string>
#include <iostream>
#include "PacketBuffer.h"
#include "MonitoringDatas.h"

bool connectSession(Session* sess, const char* serverIP, int serverPort)
{
	sess->socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sess->socket == INVALID_SOCKET)
	{
		std::cerr << "socket() failed, err=" << WSAGetLastError() << "\n";
		return false;
	}

	u_long on = 1;
	ioctlsocket(sess->socket, FIONBIO, &on);

	linger lingerOption;
	lingerOption.l_onoff = 1;  // linger 옵션 활성화
	lingerOption.l_linger = 0; // 연결 종료 시 즉시 닫힘

	setsockopt(sess->socket, SOL_SOCKET, SO_LINGER, (const char*)&lingerOption, sizeof(lingerOption));

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);

	InetPtonA(AF_INET, serverIP, &serverAddr.sin_addr);

	int ret = connect(sess->socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	//InterlockedIncrement(&connectCnt);
	InterlockedIncrement(&connectTryCnt);

	if (ret == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK)
	{
		std::cerr << "connect() error, err=" << WSAGetLastError() << "\n";
		closesocket(sess->socket);
		sess->socket = INVALID_SOCKET;
		InterlockedIncrement(&connectFail);
		//InterlockedDecrement(&connectCnt);
		return false;
	}

	sess->isSession = true;
	sess->isConnected = false;
	return true;
}

bool netProc_Recv(Session* session)
{
	int bytesReceived = 0;
	int directEnqueueSize = session->recvQ.DirectEnqueueSize();
	bytesReceived = recv(session->socket, session->recvQ.GetRearBuffer(), directEnqueueSize, 0);

	if (bytesReceived == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			return true;
		}
		else if (WSAGetLastError() == WSAECONNRESET)
		{
			if(!session->checkHeartBeat)
				InterlockedIncrement(&disconnectFail);
			return false;
		}
		else
		{
			printf("client %lld recv error : %d\n", session->id, WSAGetLastError());
			return false;
		}
	}
	else if (bytesReceived == 0)
	{
		if (!session->checkHeartBeat)
			InterlockedIncrement(&disconnectFail);
		return false;
	}

	session->recvQ.MoveRear(bytesReceived);

	while (1)
	{
		if (session->recvQ.GetUseSize() < sizeof(NetWorkHeader))
			break;

		NetWorkHeader netHeader;
		int netHeaderPeek = session->recvQ.Peek((char*)&netHeader, sizeof(NetWorkHeader));

		if (netHeader.code != Protocol_Code)
		{
			InterlockedIncrement(&fixedCodeErrorCnt);
			return false;
		}

		if (session->recvQ.GetUseSize() < sizeof(NetWorkHeader) + netHeader.len)
			break;

		//PacketBuffer packet;
		//session->recvQ.MoveFront(sizeof(NetWorkHeader));
		//session->recvQ.Dequeue(packet.GetBufferPtr(), netHeader.len);
		//packet.MoveWritePos(netHeader.len);
		PacketBuffer packet;
		session->recvQ.Dequeue(packet.GetBufferPtr(), netHeader.len + sizeof(NetWorkHeader));
		packet.MoveWritePos(netHeader.len + sizeof(NetWorkHeader));
		Decode(&packet);

		packet.DequeueData((char*)&netHeader, sizeof(netHeader));

		//payload가 되겠지?? 네트워크 헤더를 뜯으면
		int debug = GetPayLoadCheckSum(&packet);
		if (netHeader.checkSum != GetPayLoadCheckSum(&packet))
		{
			InterlockedIncrement(&checkSumErrorCnt);
			return false;
		}

		if (!RecvProcess(session, &packet))
			return false;
	}

	return true;
}

bool RecvProcess(Session* session, PacketBuffer* packet)
{
	ContentHeader conHeader;
	packet->DequeueData((char*)&conHeader, sizeof(ContentHeader));

	//printf("--packettype : %d--\n", conHeader.type);

	switch (conHeader.type)
	{
	case stPacket_Chat_Client_CreateCharacter:
		return Packet_CreateCharacter(session, packet);
		break;
	case stPacket_Chat_Client_LocalChat:
		return Packet_LocalChat(session, packet);
		break;
	case stPacket_Chat_Client_MoveStopOk:
		return Packet_MoveStopOk(session, packet);
		break;
	case stPacket_Chat_Client_ChatComplete:
		return Packet_ChatComplete(session, packet);
		break;
	default:
		InterlockedIncrement(&packetTypeErrorCnt);
		return false;
		break;
	}

	return true;
}

bool netProc_Send(Session* session)
{
	int bytesSent = 0;
	int directDequeueSize = session->sendQ.DirectDequeueSize();

	if (directDequeueSize == 0)
		return true;

	bytesSent = send(session->socket, session->sendQ.GetFrontBuffer(), directDequeueSize, 0);
	session->rtt = timeGetTime();
	if (bytesSent == SOCKET_ERROR)
	{
		int errCode = WSAGetLastError();
		if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			return true;
		}
		else if (WSAGetLastError() == WSAECONNRESET)
		{
			if (!session->checkHeartBeat)
				InterlockedIncrement(&disconnectFail);
			return false;
		}
		else
		{
			printf("[ERROR] send failed (SessionID: %llu, Error Code: %d)\n", session->id, errCode);
			return false;
		}	
	}
	else if (bytesSent > 0)
	{
		session->sendQ.MoveFront(bytesSent);
	}
	else if(bytesSent == 0)
	{
		if (!session->checkHeartBeat)
			InterlockedIncrement(&disconnectFail);
		return false;
	}
	return true;
}

bool Packet_CreateCharacter(Session* session, PacketBuffer* packet)
{
	//printf("createCharater received\n");

	if (packet->GetDataSize() != 16)
		InterlockedIncrement(&packetlenErrorCnt);

	if (session->isCharacterActive)
		InterlockedIncrement(&charOverlappedCnt);

	unsigned long long characterID;
	int shX, shY;

	*packet >> characterID;
	*packet >> shX;
	*packet >> shY;

	session->id = characterID;
	session->shX = shX;
	session->shY = shY;

	//printf("shX : %d / shY : %d\n", shX, shY);

	session->isCharacterActive = true;

	session->CurSector.iX = session->shX / dfSECTOR_SIZE_X;
	session->CurSector.iY = session->shY / dfSECTOR_SIZE_Y;
	session->OldSector.iX = session->CurSector.iX;
	session->OldSector.iY = session->CurSector.iY;

	AddSector(session->CurSector, session);

	//DebugBreak();

	return true;
}

bool Packet_LocalChat(Session* session, PacketBuffer* packet)
{
	//printf("----------------------------localChat received------------------------------\n");
	if (!session->isCharacterActive)
		InterlockedIncrement(&forDebug);

	//unsigned long long characterID;
	//BYTE chatLen;
	//*packet >> characterID;
	//*packet >> chatLen;

	//std::string chatMessage;
	//chatMessage.resize(chatLen);

	//packet->DequeueData(&chatMessage[0], chatLen);

	////여기서 채팅 판단
	//
	//auto it = std::find
	//(
	//	g_randomMessages.begin(),
	//	g_randomMessages.end(),
	//	chatMessage
	//);

	//if (it == g_randomMessages.end()) 
	//{
	//	printf("no exist chatMessageRecv : %llu\n", session->id);
	//	InterlockedIncrement(&wrongChatCnt);
	//	return false;
	//}

	InterlockedIncrement(&totalMessageRecv);

	/*if (trustMode == 0)
	{
		int idx = int(it - g_randomMessages.begin());


		if (!session->isReadyForChat)
			return true;

		int ret1 = InterlockedDecrement(&session->messageCount[idx]);
		if (ret1 < 0)
		{
			InterlockedIncrement(&session->messageCount[idx]);
			InterlockedAdd(&forDebug, ret1);
		}
		int ret2 = InterlockedDecrement(&chatCompleteCnt);
	}*/
	//printf("chatcompleteCnt 감소 : %d\n", ret2);

	return true;
}

bool Packet_MoveStopOk(Session* session, PacketBuffer* packet)
{
	if (!session->isCharacterActive)
		InterlockedIncrement(&forDebug);

	if (packet->GetDataSize() != 0)
		InterlockedIncrement(&packetlenErrorCnt);
	//printf("movestopOk received\n");

	//session->moveStopRecv = true;
	if (session->moveStopRecv)
	{
		session->moveStopRecv = false;
	}
	else
	{
		InterlockedIncrement(&MoveStopOverlapError);
	}

	InterlockedDecrement(&moveStopCnt);

	if(trustMode ==0)
		InterlockedIncrement(&totalMoveStopRecv);

	return true;
}

bool Packet_ChatComplete(Session* session, PacketBuffer* packet)
{
	DWORD now = timeGetTime();

	if (!session->isCharacterActive)
		InterlockedIncrement(&forDebug);

	if (packet->GetDataSize() != 0)
		InterlockedIncrement(&packetlenErrorCnt);

	//printf("ChatComplete received\n");
	//InterlockedDecrement(&chatCompleteCnt);
	InterlockedDecrement(&chatPlayerCnt);
	
	DWORD rtt = now - session->rtt;

	rtt = (rtt > 40) ? (rtt - 40) : 0;

	RTTData.Add(rtt);
	//RTTData.Add(now - session->rtt);

	if (!session->chatSent)
	{
		printf("wrong chat complete : %llu\n", session->id);
	}
	else
	{
		session->chatSent = false;
	}

	return true;
}

void SendPacket_Unicast(Session* session, PacketBuffer* packet)
{
	if (session->sendQ.GetFreeSize() < packet->GetDataSize())
		printf("session ID : %llu sendQ no size\n", session->id);

	Encode(packet);
	session->sendQ.Enqueue(packet->GetBufferPtr(), packet->GetDataSize());
}

void SetCheckSum(PacketBuffer* buffer)
{
	int dataSize = buffer->GetDataSize();
	char* start = buffer->GetBufferPtr();
	char* ptr = start + sizeof(NetWorkHeader);
	char* end = start + dataSize;
	char* check = buffer->GetBufferPtr() + 4;
	BYTE checkSum = 0;

	for (; ptr < end; ++ptr)
	{
		checkSum += *ptr;
	}

	*check = checkSum;
}

int GetPayLoadCheckSum(PacketBuffer* buffer)
{
	int dataSize = buffer->GetDataSize();
	char* start = buffer->GetBufferPtr() + sizeof(NetWorkHeader);
	char* ptr = start;
	char* end = start + dataSize;
	//ptr += sizeof(NetWorkHeader);
	BYTE checkSum = 0;

	int debug = end - ptr;

	for (; ptr < end; ++ptr)
	{
		checkSum += *ptr;
	}
	return checkSum;
}

void Encode(PacketBuffer* buffer)
{
	BYTE fixedKey = Fixed_Code;
	BYTE randomKey = *(buffer->GetBufferPtr() + 3);
	int dataSize = buffer->GetDataSize();
	char* start = buffer->GetBufferPtr();
	char* ptr = start;
	char* end = start + dataSize;
	ptr += 4;

	BYTE prevP = 0;
	BYTE prevE = 0;
	int  offset = 1;

	for (; ptr < end; ++ptr, ++offset)
	{
		BYTE p = *ptr ^ (prevP + randomKey + offset);

		BYTE e = p ^ (prevE + fixedKey + offset);

		*ptr = e;

		prevP = p;
		prevE = e;
	}
}

void Decode(PacketBuffer* buffer)
{
	BYTE fixedKey = Fixed_Code;
	BYTE randomKey = *(buffer->GetBufferPtr() + 3);
	int dataSize = buffer->GetDataSize();
	char* start = buffer->GetBufferPtr();
	char* ptr = start;
	char* end = start + dataSize;
	ptr += 4;

	BYTE prevE = 0;
	BYTE prevP = 0;
	int  offset = 1;

	for (; ptr < end; ++ptr, ++offset)
	{
		BYTE c = *ptr;
		BYTE p = c ^ (prevE + fixedKey + offset);
		BYTE d = p ^ (prevP + randomKey + offset);
		*ptr = d;
		prevE = c;
		prevP = p;
	}
}
