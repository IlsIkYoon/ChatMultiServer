#pragma once

class PacketBuffer
{
public:
	enum en_PACKET
	{
		eBUFFER_DEFAULT = 1400
	};

	PacketBuffer();
	PacketBuffer(int bufferSize);
	virtual ~PacketBuffer();

	void Clear();
	int GetBufferSize() const { return p_BufferSize; }
	int GetDataSize() const { return p_WritePos-p_ReadPos; }
	char* GetReadPtr() { return p_Buffer + p_ReadPos; }
	char* GetBufferPtr() { return p_Buffer; }

	int MoveWritePos(int size);
	int MoveReadPos(int size);

	bool GetError() const { return p_Error; }
	bool ResizeBuffer(int newSize);

	// Data Write operator <<
	PacketBuffer& operator<<(unsigned char value);
	PacketBuffer& operator<<(char value);

	PacketBuffer& operator<<(short value);
	PacketBuffer& operator<<(unsigned short value);

	PacketBuffer& operator<<(int value);
	PacketBuffer& operator<<(unsigned int value);

	PacketBuffer& operator<<(long value);
	PacketBuffer& operator<<(unsigned long value);

	PacketBuffer& operator<<(float value);

	PacketBuffer& operator<<(double value);
	PacketBuffer& operator<<(unsigned __int64 value);

	// Data Read operator >>
	PacketBuffer& operator>>(unsigned char& value);
	PacketBuffer& operator>>(char& value);

	PacketBuffer& operator>>(short& value);
	PacketBuffer& operator>>(unsigned short& value);

	PacketBuffer& operator>>(int& value);
	PacketBuffer& operator>>(unsigned int& value);

	PacketBuffer& operator>>(long& value);
	PacketBuffer& operator>>(unsigned long& value);

	PacketBuffer& operator>>(float& value);

	PacketBuffer& operator>>(double& value);
	PacketBuffer& operator>>(unsigned __int64& value);

	int DequeueData(char* dest, int size);
	int EnqueueData(const char* src, int size);

protected:
	char* p_Buffer;
	int p_BufferSize;
	int p_DataSize;
	int p_ReadPos;
	int p_WritePos;
	int resizeCnt;
	bool p_Error;
};