#include "LoginServer.h"



int main()
{
	bool serverRetval;

	serverRetval = LoginServer();

	if (serverRetval == false)
	{
		printf("Server Error!!!!!!!!!\n");
	}

	return 0;
}