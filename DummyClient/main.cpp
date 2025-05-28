#include "resource.h"
#include "DummyClient.h"
#include "CrashDump.h"


int main()
{
	procademy::CCrashDump dump;

	bool retval;
	retval = DummyClient();
	if (retval == false)
	{
		// 오류 정보 발생
	}


	return 0;
}