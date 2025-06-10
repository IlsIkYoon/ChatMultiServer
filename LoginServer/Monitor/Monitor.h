#pragma once
#include "Resource/LoginServerResource.h"


class CMonitor
{
public:
	unsigned long long loginSuccessCount;
	unsigned long long loginFailedCount;

	



public:
	//출력함수 
	void ConsolPrintAll();

	void ConsolPrintLoginCount();
	void ConsolPrintUserCount();



};