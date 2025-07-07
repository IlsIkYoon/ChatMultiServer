#pragma once
#include "Resource/LoginServerResource.h"


class CMonitor_delete
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