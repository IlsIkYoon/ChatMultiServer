#include "RingBuffer.h"
#include <string>

RingBuffer::RingBuffer() : buffer(nullptr), bufferSize(0), readPos(0), writePos(0), resizeCnt(0) {}

RingBuffer::RingBuffer(int size) : buffer(new char[size]), bufferSize(size), readPos(0), writePos(0), resizeCnt(0) {}

RingBuffer::~RingBuffer()
{
	delete[] buffer;
}

void RingBuffer::Resize()
{
	if (resizeCnt >= 2) return;

	int newSize = bufferSize * 3 / 2;

	char* newBuffer = new char[newSize];
	int dataToCopy = GetUseSize();

	if (dataToCopy > 0)
	{
		if (readPos < writePos)
		{
			memcpy(newBuffer, buffer + readPos, dataToCopy);
		}
		else
		{
			int firstChunk = bufferSize - readPos;
			memcpy(newBuffer, buffer + readPos, firstChunk);
			memcpy(newBuffer + firstChunk, buffer, writePos);
		}
	}
	delete[] buffer;
	buffer = newBuffer;
	bufferSize = newSize;
	writePos = dataToCopy;
	readPos = 0;
	resizeCnt++;
}

int RingBuffer::GetBufferSize() const
{
	return bufferSize;
}

int RingBuffer::GetUseSize() const
{
	if (writePos >= readPos)
		return writePos - readPos;

	return bufferSize - (readPos - writePos);
}

int RingBuffer::GetFreeSize() const
{
	return bufferSize - 1 - GetUseSize();
}

int RingBuffer::Enqueue(const char* data, int size)
{
	if (size <= 0 || size > GetFreeSize()) return 0;

	for (int i = 0; i < size; i++)
	{
		buffer[writePos] = data[i];
		writePos = (writePos + 1) % bufferSize;
	}
	return size;
}

int RingBuffer::Dequeue(char* data, int size)
{
	if (size <= 0 || size > GetUseSize()) return 0;

	for (int i = 0; i < size; i++)
	{
		data[i] = buffer[readPos];
		readPos = (readPos + 1) % bufferSize;
	}

	return size;
}

int RingBuffer::Peek(char* data, int size) const
{
	if (size <= 0 || size > GetUseSize()) return 0;

	int tempPos = readPos;
	for (int i = 0; i < size; i++)
	{
		data[i] = buffer[tempPos];
		tempPos = (tempPos + 1) % bufferSize;
	}
	return size;
}

int RingBuffer::DirectEnqueueSize() const
{
	if (writePos >= readPos)
	{
		//return bufferSize - writePos - 1;
		return bufferSize - writePos - (readPos == 0 ? 1 : 0);
	}
	return readPos - writePos - 1;
}

int RingBuffer::DirectDequeueSize() const
{
	if (readPos > writePos)
	{
		return bufferSize - readPos;
	}
	else if (readPos < writePos)
	{
		return writePos - readPos;
	}
	else
	{
		return 0;
	}
}

void RingBuffer::ClearBuffer()
{
	readPos = 0;
	writePos = 0;
}

int RingBuffer::MoveFront(int size)
{
	if (size <= 0 || size > GetUseSize()) return 0;

	readPos = (readPos + size) % bufferSize;
	return size;
}

int RingBuffer::MoveRear(int size)
{
	if (size <= 0 || size > GetFreeSize()) return 0;

	writePos = (writePos + size) % bufferSize;
	return size;
}

bool RingBuffer::IsFull() const
{
	return GetFreeSize() == 0;
}

bool RingBuffer::IsEmpty() const
{
	return readPos == writePos;
}
