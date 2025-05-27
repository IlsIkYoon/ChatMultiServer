#pragma once
#include "pch.h"
#include "LFMemoryPool.h"
#include "TLSPool.h"
#include "SerializeBuf.h"



#define MASKING_VALUE17BIT 0x00007fffffffffff

#define ENQUEUE 0x00
#define DEQUEUE 0x01
#define BIT_64 64
#define UNUSED_BIT 17
#define BIT_OR_VALUE (BIT_64 - UNUSED_BIT)

#ifdef __LFDEBUG__


extern unsigned long long logMaxNum;
extern unsigned long long g_SeqNum;
extern unsigned long long g_CASERROR_SeqNum;
extern unsigned long long g_TailDefer_SeqNum;

struct LogData
{
	char funcType;
	DWORD TID;

	unsigned long long seqNum;

	ULONG_PTR firstCasParam2;
	ULONG_PTR firstCasParam3;
	ULONG_PTR SecondCasParam2;
	ULONG_PTR SecondCasParam3;

	ULONG_PTR NewNodeAddress;
	ULONG_PTR DeleteNodeAddress;

	bool CAS1Success;
	bool CAS2Success;
};

extern LogData* logArr;
extern ULONG64* CASERROR_logArr;
extern LogData* TailDefer_logArr;
#endif


template <typename T>
class LFreeQ
{
public:
	struct Node
	{
		T _data;
		Node* _next;

	};

	Node* _head;
	Node* _tail;
	long _size;
	unsigned long long _bitCount;

public:
	ThreadPool<Node> _mPool;

	LFreeQ()
	{
		_head = (Node*)_mPool.Alloc();

		_head->_next = (Node*)this;
		_head->_data = 0;
		_tail = _head;
		_size = 0;
		_bitCount = 0;

#ifdef __LFDEBUG__
		logArr = (LogData*)malloc(sizeof(LogData) * logMaxNum);
		CASERROR_logArr = (ULONG64*)malloc(sizeof(ULONG64) * (logMaxNum));
		TailDefer_logArr = (LogData*)malloc(sizeof(LogData) * (logMaxNum));
#endif
	}

	inline unsigned long long GetSize()
	{
		return _size;
	}
	inline unsigned long long GetBitCount()
	{
		return _bitCount;
	}



	inline void* PackPtr(void* ptr)
	{
		//if (ptr == this) __debugbreak();

		unsigned long long localBitCount = InterlockedIncrement(&_bitCount);
		void* retPtr;

		retPtr = (void*)((ULONG_PTR)ptr | (localBitCount << BIT_OR_VALUE));

		return retPtr;
	}

	static void* UnpackPtr(void* ptr)
	{
		void* retPtr = (void*)((ULONG_PTR)ptr & MASKING_VALUE17BIT);

		return retPtr;
	}
	inline Node* PeekFront()
	{
		return ((Node*)(UnpackPtr(_head)))->_next;
	}


	inline void Enqueue(T pData)
	{
		Node* localTail;
		Node* exchangeNode;
		Node* localTailAddress;
		Node* nextNode;
		Node* newNode;

#ifdef __LFDEBUG__
		unsigned long long localSeqNum;
		DWORD myTID;

		bool CAS1Success;
		bool CAS2Success;
		myTID = GetCurrentThreadId();
#endif

		newNode = (Node*)_mPool.Alloc();

		newNode->_data = pData;
		newNode->_next = (Node*)this;
		exchangeNode = (Node*)PackPtr(newNode); //상위 17비트는 0일 거라는 가정 하에 진행



		while (1)
		{
#ifdef __LFDEBUG__
			CAS1Success = false;
			CAS2Success = false;
#endif
			localTail = _tail;
			localTailAddress = (Node*)UnpackPtr(localTail);
			nextNode = localTailAddress->_next;
			
			Node* packTailNext = (Node*)PackPtr(nextNode);

			if (nextNode == nullptr)
			{
				__debugbreak();
				continue;
			}

			if (nextNode != (Node*)this) //여기서 null인지에 대한 판단이 왜 들어가야 하는거지 ??
			{
				if (localTailAddress == nextNode)
					__debugbreak();
				if (InterlockedCompareExchangePointer((PVOID*)&_tail, packTailNext, localTail) == localTail)
				{
#ifdef __LFDEBUG__
					__debugbreak();
					ULONG64 localTailDeferSeqNum = InterlockedIncrement(&g_TailDefer_SeqNum);
					if (localTailDeferSeqNum >= LOG_MAXNUM - 1)
					{
						InterlockedExchange(&g_TailDefer_SeqNum, 0);
						localTailDeferSeqNum = 0;
					}
					TailDefer_logArr[localTailDeferSeqNum].CAS1Success = true;
					TailDefer_logArr[localTailDeferSeqNum].funcType = ENQUEUE;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam2 = (ULONG_PTR)localTailAddress->_next;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam3 = (ULONG_PTR)localTail;
					TailDefer_logArr[localTailDeferSeqNum].TID = myTID;
#endif
				}

				continue;

			}


			if (localTailAddress == newNode)
			{
				__debugbreak();
			}

			if (InterlockedCompareExchangePointer((PVOID*)&localTailAddress->_next, newNode, (Node*)this) == (Node*)this)
			{
				
#ifdef __LFDEBUG__
				CAS1Success = true;
#endif
				InterlockedIncrement(&_size);

				if (InterlockedCompareExchangePointer((PVOID*)&_tail, exchangeNode, localTail) == localTail)
				{
#ifdef __LFDEBUG__
					CAS2Success = true;
#endif
				}
#ifdef __LFDEBUG__
				localSeqNum = InterlockedIncrement(&g_SeqNum);
				if (localSeqNum >= LOG_MAXNUM - 1)
				{
					InterlockedExchange(&g_SeqNum, 0);
					localSeqNum = 0;
				}

				logArr[localSeqNum].seqNum = localSeqNum;
				logArr[localSeqNum].TID = myTID;
				logArr[localSeqNum].funcType = ENQUEUE;
				logArr[localSeqNum].firstCasParam2 = (ULONG_PTR)exchangeNode;
				logArr[localSeqNum].firstCasParam3 = (ULONG_PTR)nullptr;
				logArr[localSeqNum].SecondCasParam2 = (ULONG_PTR)exchangeNode;
				logArr[localSeqNum].SecondCasParam3 = (ULONG_PTR)localTail;
				logArr[localSeqNum].NewNodeAddress = (ULONG_PTR)newNode;
				logArr[localSeqNum].CAS1Success = CAS1Success;
				logArr[localSeqNum].CAS2Success = CAS2Success;


				if (CAS2Success == false)
				{
					unsigned long long CasSeqNum = InterlockedIncrement(&g_CASERROR_SeqNum);



					CASERROR_logArr[CasSeqNum] = localSeqNum;

				}
#endif
				
				break;
			}


		}

	}

	T Dequeue()
	{
		Node* localHead;
		Node* localTail;
		Node* localHeadAddress;
		Node* localTailAddress;
		Node* nextNode;
		T retval;

#ifdef __LFDEBUG__
		unsigned long long localSeqNum;
		DWORD myTID;
		bool CAS1Success;
		myTID = GetCurrentThreadId();
#endif
		long localSize = InterlockedDecrement(&_size);

		if (localSize < 0)
		{
			InterlockedIncrement(&_size);
			return (T) nullptr;
		}






		while (1)
		{
#ifdef __LFDEBUG__
			CAS1Success = false;
#endif
			localHead = _head;
			localTail = _tail;

			localHeadAddress = (Node*)UnpackPtr(localHead);
			localTailAddress = (Node*)UnpackPtr(localTail);
			nextNode = localHeadAddress->_next;
			Node* tailNext = localTailAddress->_next;
			Node* packTailNext = (Node*)PackPtr(tailNext);
			Node* packHeadNext = (Node*)PackPtr(nextNode);
			
			if (localTailAddress->_next == nullptr)
			{
				continue;
			}

			if (tailNext != (Node*)this)
			{
				if (tailNext == localTailAddress)
					__debugbreak();
				if (InterlockedCompareExchangePointer((PVOID*)&_tail, packTailNext, localTail) == localTail)
				{
#ifdef __LFDEBUG__
					
					ULONG64 localTailDeferSeqNum = InterlockedIncrement(&g_TailDefer_SeqNum);
					
					if (localTailDeferSeqNum >= LOG_MAXNUM - 1)
					{
						InterlockedExchange(&g_TailDefer_SeqNum, 0);
						localTailDeferSeqNum = 0;
					}

					TailDefer_logArr[localTailDeferSeqNum].CAS1Success = true;
					TailDefer_logArr[localTailDeferSeqNum].funcType = DEQUEUE;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam2 = (ULONG_PTR)localTailAddress->_next;
					TailDefer_logArr[localTailDeferSeqNum].firstCasParam3 = (ULONG_PTR)localTail;
					TailDefer_logArr[localTailDeferSeqNum].TID = myTID;
#endif
				}
				continue;
			}


			if (nextNode == (Node*)this || nextNode == nullptr)
				continue;


			retval = nextNode->_data;

			if (InterlockedCompareExchangePointer((PVOID*)&_head, packHeadNext, localHead) == localHead)
			{

#ifdef __LFDEBUG__
				CAS1Success = true;
				localSeqNum = InterlockedIncrement(&g_SeqNum);
				if (localSeqNum >= LOG_MAXNUM - 1)
				{
					InterlockedExchange(&g_SeqNum, 0);
					localSeqNum = 0;
				}

				logArr[localSeqNum].seqNum = localSeqNum;
				logArr[localSeqNum].TID = myTID;
				logArr[localSeqNum].funcType = DEQUEUE;
				logArr[localSeqNum].firstCasParam2 = (ULONG_PTR)nextNode;
				logArr[localSeqNum].firstCasParam3 = (ULONG_PTR)localHead;
				logArr[localSeqNum].DeleteNodeAddress = (ULONG_PTR)localHead;
				logArr[localSeqNum].CAS1Success = CAS1Success;
#endif

				if (InterlockedCompareExchangePointer((PVOID*)&localHeadAddress->_next, (Node*)this, (Node*)this) == (Node*)this)
				{
					__debugbreak();
				}
				
				_mPool.Delete(localHeadAddress);
				break;
			}
		}

		return retval;
	}


	void Clear()
	{
		while (1)
		{
			T retNode;
			retNode = Dequeue();
			if (retNode == nullptr)
				break;

			retNode->DecrementUseCount();
		}

		return;
	}

};





#ifdef __LFDEBUG__
void WriteAllLogData();
#endif

