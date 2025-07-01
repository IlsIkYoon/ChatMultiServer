#pragma once
#include "pch.h"




#define MASKING_VALUE17BIT 0x00007fffffffffff
#define RETURN_NEXTVALUE 0x0f0f0f0f0f0f0f0f

#define NODECOUNT 100

#ifdef __LFDEBUG__
extern std::list<void*> _debugSave;

extern CRITICAL_SECTION g_Lock;

extern unsigned long CreateCount;
extern unsigned long DeleteCount;

#endif
template <typename T>
class LFreeMemoryPool
{
	struct Node
	{
		void* _data;
		Node* _next;
	};

	Node* _top;
	LONG _msize;
	LONG _madeCount;
	unsigned long long _count;

public:

	LFreeMemoryPool()
	{
		_top = nullptr;
		_msize = 0;
		_madeCount = 0;
		_count = 0;
	}

	void Delete(void* pNode) //스택의 push동작
	{
		Node* exchangeNode;
		Node* localTop;
		Node* newNode;

		unsigned long long localCount;

		newNode = (Node*)pNode;
		localCount = InterlockedIncrement(&_count);
		exchangeNode = (Node*)((ULONG_PTR)newNode | (localCount << (64 - 17))); //상위 17비트는 0일 거라는 가정 하에 진행

		while (1)
		{
			localTop = _top;
			newNode->_next = localTop;

			if (InterlockedCompareExchangePointer((PVOID*)&_top, exchangeNode, localTop) == localTop)
			{
				InterlockedIncrement(&_msize);
				break;
			}
		}
	}

	void* Alloc() //스택의 pop 동작
	{
		Node* localTop;
		Node* realTopAdd;
		Node* retNode;

		while (1)
		{
			localTop = _top;
			if (localTop == nullptr)
			{
				//새로 할당해서 주고 return
				Node* newNode = new Node;
				_InterlockedIncrement(&_madeCount);
				return (void*)newNode;
			}

			realTopAdd = (Node*)((ULONG_PTR)localTop & MASKING_VALUE17BIT);

			Node* nextNode = realTopAdd->_next;

			if (InterlockedCompareExchangePointer((PVOID*)&_top, nextNode, localTop) == localTop)
			{
				retNode = (Node*)((ULONG_PTR)localTop & MASKING_VALUE17BIT);
				InterlockedDecrement(&_msize);

				return (void*)retNode;
				break;
			}
		}
	}
};

template <typename T>
class LFreeStack
{
	struct Node
	{
		T _data;
		Node* _next;
	};
	Node* _top;
	LONG _msize;
	LONG _madeCount;
	unsigned long long _count;
	LFreeMemoryPool<T> _LFMemoryPool;


public:

	LFreeStack()
	{
		_top = nullptr;
		_msize = 0;
		_madeCount = 0;
		_count = 0;
	}
	~LFreeStack()
	{
		/* 소멸자에 관해선 조금만 뒤에 생각해보자
		//여기서도 노드들 전부 삭제하는 방법이 필요한데..
		while (1)
		{
			if (_top == nullptr)
				break;


			//태그를 제거해야 함
			Node* rpos;
			rpos = (Node*)((ULONG_PTR)_top & MASKING_VALUE17BIT);
			_top = rpos->_next;

			Node* deleteNode;
			deleteNode = rpos->_data;
			while (deleteNode != nullptr)
			{
				Node* deleteRpos;
				deleteRpos = deleteNode;
				deleteNode = deleteNode->_next;

#ifdef __LFDEBUG__
				EnterCriticalSection(&g_Lock);
				_debugSave.remove(deleteRpos);
				LeaveCriticalSection(&g_Lock);
				InterlockedIncrement(&DeleteCount);
#endif


				delete deleteRpos;
			}
			_msize--;
		}
		*/
	}

	LONG GetSize()
	{
		return _msize;	
	}

	void Delete(T pData) //스택의 push동작
	{
		Node* exchangeNode;
		Node* localTop;
		Node* newNode;

		unsigned long long localCount;

		newNode = (Node*)_LFMemoryPool.Alloc(); 
		newNode->_data = pData;
		localCount = InterlockedIncrement(&_count);
		exchangeNode = (Node*)((ULONG_PTR)newNode | (localCount << (64 - 17))); //상위 17비트는 0일 거라는 가정 하에 진행


		while (1)
		{
			localTop = _top;
			newNode->_next = localTop;

			if (InterlockedCompareExchangePointer((PVOID*)&_top, exchangeNode, localTop) == localTop)
			{
				InterlockedIncrement(&_msize);


				break;
			}

		}

	}

	T Alloc() //스택의 pop 동작
	{
		Node* localTop;
		Node* realTopAdd;
		Node* retNode;
		T retData;

		if (_msize == 0)
		{
			return (T)nullptr;
		}

		while (1)
		{
			localTop = _top;
			if (localTop == nullptr)
			{
				return (T)nullptr;
			}

			realTopAdd = (Node*)((ULONG_PTR)localTop & MASKING_VALUE17BIT);


			Node* nextNode = realTopAdd->_next;


			if (InterlockedCompareExchangePointer((PVOID*)&_top, nextNode, localTop) == localTop)
			{
				retNode = (Node*)((ULONG_PTR)localTop & MASKING_VALUE17BIT);
				InterlockedDecrement(&_msize);

				retData = retNode->_data;
				_LFMemoryPool.Delete(retNode);

				return retData;
			}

		}

	}

};

template <typename T>
class TMemoryPool
{
	struct Node
	{
		T _data;
		Node* _next;
	};
	Node* _top;

	Node* _freeTop;
	bool _free;

	LONG _msize;

public:
	static LFreeStack<T*> _gMemoryPool;

	TMemoryPool()
	{
		_top = nullptr;
		_freeTop = nullptr;
		_free = false;
		_msize = 0;

	}

	~TMemoryPool()
	{
		

		if (_freeTop != nullptr)
		{
			_gMemoryPool.Delete(&_freeTop->_data);
			_freeTop = nullptr;
		}
		while (1)
		{
			if (_top == nullptr)
				break;
			Node* rpos;
			rpos = _top;
			_top = _top->_next;
#ifdef __LFDEBUG__
			EnterCriticalSection(&g_Lock);
			_debugSave.remove(rpos);
			LeaveCriticalSection(&g_Lock);
			InterlockedIncrement(&DeleteCount);
#endif
			delete rpos;
			_msize--;
		}
	}

	inline LONG GetSize()
	{
		return _msize;
	}

	void SwapList()
	{
		if (_freeTop != nullptr)
		{
			_gMemoryPool.Delete(&_freeTop->_data);
			_freeTop = nullptr;
		}

		_freeTop = _top;
		_top = nullptr;
		_msize = 0;

	}

	void Delete(void* pNode) //스택의 push동작
	{

		if (_msize >= NODECOUNT) {
			SwapList();
		}

		Node* newNode;

		newNode = (Node*)pNode;

		_msize++;


		newNode->_next = _top;
		_top = newNode;


	}

	void* Alloc() //스택의 pop 동작
	{
		Node* retNode;

		if (_msize == 0)
		{
			if (_freeTop != nullptr)
			{
				_top = _freeTop;
				_freeTop = nullptr;
				_msize = NODECOUNT;
			}
			else
			{
				Node* nodeArr = (Node*)_gMemoryPool.Alloc();
				if (nodeArr == nullptr)
				{
					
					retNode = new Node;
#ifdef __LOGDEBUG__
					EnterCriticalSection(&g_Lock);
					_debugSave.push_back(retNode);
					LeaveCriticalSection(&g_Lock);

					InterlockedIncrement(&CreateCount);
#endif
					return retNode;
				}
				_top = nodeArr;
				_msize = NODECOUNT;
			}
		}
		if (_top == nullptr)
		{
			__debugbreak();
		}
		retNode = _top;
		_top = _top->_next;
		_msize--;

		return retNode;
	}


};


template<typename T>
class ThreadPool
{
public:

	
	static thread_local TMemoryPool<T> localPool;

	ThreadPool() = default;


	//여기에 자료구조들이 다 들어 있고?

	void* Alloc()
	{
		return localPool.Alloc();
	}

	void Delete(void* node)
	{
		localPool.Delete(node);
	}



};


template<typename T>
LFreeStack<T*> TMemoryPool<T>::_gMemoryPool;

template <typename T>
thread_local TMemoryPool<T> ThreadPool<T>::localPool;