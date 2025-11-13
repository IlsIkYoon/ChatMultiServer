#include "Profiler.h"

//#ifdef __PROFILE__
thread_local ProfilerMap g_ProfileMap;
CRITICAL_SECTION g_ProfilerWriteLock;
unsigned long g_lockInit = (unsigned long)LockStatus::LOCK_UNINITIALIZED;






//#endif