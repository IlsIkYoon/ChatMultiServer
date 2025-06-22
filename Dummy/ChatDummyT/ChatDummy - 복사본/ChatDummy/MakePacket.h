#pragma once

#include <Windows.h>
#include "PacketBuffer.h"

void mpMoveStart(PacketBuffer* packet, BYTE byDir, int shX, int shY);

void mpMoveStop(PacketBuffer* packet, BYTE byDir, int shX, int shY);

void mpLocalChat(PacketBuffer* packet, BYTE len, const char* message);

void mpChatEnd(PacketBuffer* packet);

void mpHeartBeat(PacketBuffer* packet);