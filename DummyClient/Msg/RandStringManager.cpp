#include "RandStringManager.h"

CRandStringManager g_RandStringManager;

CRandStringManager::CRandStringManager()
{
	for (int i = 0; i < STRINGCOUNT; i++)
	{
		randStringArr[i] = new char[i + 10];
		stringLenArr[i] = i + 10;
		for (int j = 0; j < stringLenArr[i] - 1; j++)
		{
			randStringArr[i][j] = rand() % 100;
		}
		randStringArr[i][stringLenArr[i] - 1] = NULL;
	}
}


void CRandStringManager::GetRandString(char** outString, int* outLen)
{
	int randIndex = rand() % STRINGCOUNT;
	*outString = randStringArr[randIndex];
	*outLen = stringLenArr[randIndex];
}