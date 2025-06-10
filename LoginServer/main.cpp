#include "LoginServer.h"
#include "Resource/LoginServerResource.h"


int main()
{

	if (LoginServer() == false)
	{
		printf("Server Error!!!!!!!!!\n");
	}

	return 0;
}