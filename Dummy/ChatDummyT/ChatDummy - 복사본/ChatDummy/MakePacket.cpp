#include "Network.h"
#include "MakePacket.h"
#include "Protocol.h"

void mpMoveStart(PacketBuffer* packet, BYTE byDir, int shX, int shY)
{
	NetWorkHeader netHeader;
	netHeader.code = Protocol_Code;
	netHeader.len = 11;
	netHeader.randkey = Random_Key;
	netHeader.checkSum = -1;

	ContentHeader conHeader;
	//conHeader.len = 9;
	conHeader.type = stPacket_Client_Chat_MoveStart;
	packet->Clear();

	packet->EnqueueData((char*)&netHeader, sizeof(netHeader));
	packet->EnqueueData((char*)&conHeader, sizeof(conHeader));

	*packet << byDir;
	*packet << shX;
	*packet << shY;
	SetCheckSum(packet);
}

void mpMoveStop(PacketBuffer* packet, BYTE byDir, int shX, int shY)
{
	NetWorkHeader netHeader;
	netHeader.code = Protocol_Code;
	netHeader.len = 11;
	netHeader.randkey = Random_Key;
	netHeader.checkSum = -1;

	ContentHeader conHeader;
	//conHeader.len = 9;
	conHeader.type = stPacket_Client_Chat_MoveStop;
	packet->Clear();

	packet->EnqueueData((char*)&netHeader, sizeof(netHeader));
	packet->EnqueueData((char*)&conHeader, sizeof(conHeader));

	*packet << byDir;
	*packet << shX;
	*packet << shY;
	SetCheckSum(packet);
}

void mpLocalChat(PacketBuffer* packet, BYTE len, const char* message)
{
	NetWorkHeader netHeader;
	netHeader.code = Protocol_Code;
	netHeader.len = len+3;
	netHeader.randkey = Random_Key;
	netHeader.checkSum = -1;

	ContentHeader conHeader;
	//conHeader.len = len+1;
	conHeader.type = stPacket_Client_Chat_LocalChat;
	packet->Clear();

	packet->EnqueueData((char*)&netHeader, sizeof(netHeader));
	packet->EnqueueData((char*)&conHeader, sizeof(conHeader));

	*packet << len;
	packet->EnqueueData(message, len);
	SetCheckSum(packet);
}

void mpChatEnd(PacketBuffer* packet)
{
	NetWorkHeader netHeader;
	netHeader.code = Protocol_Code;
	netHeader.len = 2;
	netHeader.randkey = Random_Key;
	netHeader.checkSum = -1;

	ContentHeader conHeader;
	//conHeader.len = 0;
	conHeader.type = stPacket_Client_Chat_ChatEnd;
	packet->Clear();

	packet->EnqueueData((char*)&netHeader, sizeof(netHeader));
	packet->EnqueueData((char*)&conHeader, sizeof(conHeader));
	SetCheckSum(packet);
}

void mpHeartBeat(PacketBuffer* packet)
{
	NetWorkHeader netHeader;
	netHeader.code = Protocol_Code;
	netHeader.len = 2;
	netHeader.randkey = Random_Key;
	netHeader.checkSum = -1;

	ContentHeader conHeader;
	//conHeader.len = 0;
	conHeader.type = stPacket_Client_Chat_HeartBeat;
	packet->Clear();

	packet->EnqueueData((char*)&netHeader, sizeof(netHeader));
	packet->EnqueueData((char*)&conHeader, sizeof(conHeader));
	SetCheckSum(packet);
}
