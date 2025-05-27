#include "pch.h"
#include "RingBuffer.h"


RingBuffer::RingBuffer()
{
	_buf = (char*)malloc(DefaultBufSize);
	_rear = 0;
	_front = 0;
	_bufSize = DefaultBufSize;
}
RingBuffer::RingBuffer(int size)
{
	_buf = (char*)malloc(size);
	_rear = 0;
	_front = 0;
	_bufSize = size;

}
RingBuffer::~RingBuffer()
{
	delete _buf;
}



bool RingBuffer::IsEmpty()
{
	if (_rear == _front) return true;

	return false;
}
bool RingBuffer::IsFull()
{


	if ((_rear == _bufSize - 1) && (_front == 0)) return true;
	if (_rear == _front - 1) return true;


	return false;
}

unsigned int RingBuffer::GetSizeTotal()
{
	return _bufSize - 1;

}
unsigned int RingBuffer::GetSizeFree()
{
	return GetSizeTotal() - GetSizeUsed();
}
unsigned int RingBuffer::GetSizeUsed()
{
	int localRear = _rear;
	int localFront = _front;

	if (localRear > localFront) return localRear - localFront;
	if (localRear == localFront) return 0;
	if (localFront > localRear) return _bufSize - localFront + localRear;

	else __debugbreak(); return 0;
}
unsigned int RingBuffer::GetSizeUsed(int* outLocalRear, int* outLocalFront)
{
	int localRear = _rear;
	int localFront = _front;
	*outLocalRear = localRear;
	*outLocalFront = localFront;

	if (localRear > localFront) return localRear - localFront;
	if (localRear == localFront) return 0;
	if (localFront > localRear) return _bufSize - localFront + localRear;

	else __debugbreak(); return 0;
}

unsigned int RingBuffer::GetDirectEnqueSize()
{
	int localFront = _front;
	int localRear = _rear;
	int localBufSize = _bufSize;


	//if (IsFull()) return 0; 
	if ((localRear == localBufSize - 1) && (localFront == 0)) return 0;
	if (localRear == localFront - 1) return 0;



	if (localFront > localRear) return localFront - localRear - 1;
	if (localFront == localRear)
	{
		if (localRear == 0)
		{
			return GetSizeTotal();
		}

		return localBufSize - localRear;

	}

	if (localRear > localFront)
	{
		if (localFront == 0) return localBufSize - localRear - 1;

		//if (_bufSize - _rear == 0) return 1; //front가 0인 상황은 isFull에서 걸러짐

		return localBufSize - localRear;

	}

	__debugbreak();
	return 0;
}
unsigned int RingBuffer::GetDirectDequeSize()
{
	int localRear = _rear;
	int localFront = _front;

	if (localFront == localRear) return 0;
	if (localRear > localFront) return localRear - localFront;
	if (localFront > localRear)
	{
		//if()

		return _bufSize - localFront;
	}

	__debugbreak();
	return 0;
}
unsigned int RingBuffer::GetDirectDequeSize(int* outLocalRear, int* outLocalFront)
{
	int localRear = _rear;
	int localFront = _front;

	*outLocalRear = localRear;
	*outLocalFront = localFront;

	if (localFront == localRear) return 0;
	if (localRear > localFront) return localRear - localFront;
	if (localFront > localRear)
	{
		//if()

		return _bufSize - localFront;
	}

	__debugbreak();
	return 0;
}



void RingBuffer::MoveRear(int val)
{
	//int localRear = _rear;
	_rear = (_rear + val) % _bufSize;
}
void RingBuffer::MoveFront(int val)
{

	_front = (_front + val) % _bufSize;


}






bool RingBuffer::Enqueue(char* src, unsigned int dataSize, unsigned int* enqueResult)
{

	if (IsFull())
	{
		*enqueResult = 0;
		return false;
	}


	int Data;
	int out;

	if (GetSizeFree() < dataSize) Data = GetSizeFree();
	else { Data = dataSize; }

	out = Data;

	if ((int)GetDirectEnqueSize() >= Data)
	{
		memcpy(&_buf[_rear], src, Data);
		MoveRear(Data);

	}
	else
	{
		int dir = GetDirectEnqueSize();
		memcpy(&_buf[_rear], src, dir);
		Data -= dir;
		_rear = 0;
		memcpy(&_buf[_rear], &src[dir], Data);
		MoveRear(Data);

	}

	*enqueResult = out;
	return true;

}
bool RingBuffer::Dequeue(char* dest, unsigned int dataSize, unsigned int* dequeResult)
{
	if (IsEmpty())
	{
		*dequeResult = 0;
		return false;
	}

	int Data;
	int out;

	//if (GetSizeUsed() < dataSize) Data = GetSizeUsed();
	if (GetSizeUsed() < dataSize) __debugbreak();
	else { Data = dataSize; }

	out = Data;

	if ((int)GetDirectDequeSize() >= Data)
	{
		memcpy(dest, &_buf[_front], Data);
		MoveFront(Data);

	}
	else
	{
		int dqDir = GetDirectDequeSize();
		memcpy(dest, &_buf[_front], dqDir);
		Data -= dqDir;
		_front = 0;
		memcpy(&dest[dqDir], &_buf[_front], Data);
		MoveFront(Data);

	}

	*dequeResult = out;
	return true;

}



char* RingBuffer::GetRear()
{
	return &_buf[_rear];
}
char* RingBuffer::GetFront()
{
	return &_buf[_front];
}


bool RingBuffer::Peek(char* dest, unsigned int dataSize, unsigned int* dequeResult)
{
	if (IsEmpty())
	{
		*dequeResult = 0;
		return false;
	}

	int Data;
	int out;

	if (GetSizeUsed() < dataSize) Data = GetSizeUsed();
	else { Data = dataSize; }

	out = Data;

	if ((int)GetDirectDequeSize() >= Data)
	{
		memcpy(dest, &_buf[_front], Data);

	}
	else
	{
		int dqDir = GetDirectDequeSize();
		memcpy(dest, &_buf[_front], dqDir);
		Data -= dqDir;
		memcpy(&dest[dqDir], &_buf[0], Data);

	}

	*dequeResult = out;
	return true;

}

char* RingBuffer::GetBufferStart()
{
	return &_buf[0];
}


void RingBuffer::BufferClear()
{
	_front = 0;
	_rear = 0;

}