#pragma once
#include "resource.h"



class CRandStringManager
{
public:
	char* randStringArr[STRINGCOUNT];
	int stringLenArr[STRINGCOUNT];

	CRandStringManager();
	void GetRandString(char** outString, int* outLen);
};