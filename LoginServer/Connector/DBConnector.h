#pragma once
#include "Resource/LoginServerResource.h"



class CDBConnector 
{


public:

	bool LoginDataRequest(CPacket* message, ULONG64 characterKey);


};