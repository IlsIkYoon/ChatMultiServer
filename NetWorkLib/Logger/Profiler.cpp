#include "pch.h"
#include "Profiler.h"

std::list<CProfilerMap*> g_ProfileMapList;
std::mutex g_ProfileMapListLock;

bool WriteAllProfileData()
{
	std::lock_guard guard(g_ProfileMapListLock);

	for (auto it : g_ProfileMapList)
	{
		std::lock_guard guard2(it->profilerMapLock);
		it->SaveProfile();
	}
	printf("Profile Saved !!!!\n");

	return true;
}

bool ResetAllProfileDate()
{
	std::lock_guard guard(g_ProfileMapListLock);

	for (auto it : g_ProfileMapList)
	{
		it->ResetData();
	}

	printf("Profile Reset !!!!\n");

	return true;
}