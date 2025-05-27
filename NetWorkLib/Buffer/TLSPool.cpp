#include "pch.h"
#include "TLSPool.h"
#include "NetworkManager/NetWorkManager.h"

#ifdef __LFDEBUG__
std::list<void*> _debugSave;
unsigned long CreateCount = 0;
unsigned long DeleteCount = 0;
CRITICAL_SECTION g_Lock;
#endif

