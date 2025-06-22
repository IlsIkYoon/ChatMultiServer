#pragma once

struct MonitoringDataStats
{
	int minValue;
	int maxValue;
	int64_t sum;
	int count;
	SRWLOCK lock;
	MonitoringDataStats()
		:minValue(INT_MAX), maxValue(INT_MIN), sum(0), count(0), lock(SRWLOCK_INIT)
	{
	}

	void Add(int value)
	{
		AcquireSRWLockExclusive(&lock);

		if (value < minValue) minValue = value;
		if (value > maxValue) maxValue = value;
		sum += value;
		count++;

		ReleaseSRWLockExclusive(&lock);
	}

	void Reset()
	{
		AcquireSRWLockExclusive(&lock);

		minValue = INT_MAX;
		maxValue = INT_MIN;
		sum = 0;
		count = 0;

		ReleaseSRWLockExclusive(&lock);
	}

	void GetSnapshot(int& outMin, int& outMax, int& outAvg)
	{
		AcquireSRWLockShared(&lock);

		outMin = minValue;
		outMax = maxValue;
		outAvg = (count > 0) ? static_cast<int>(sum) / count : 0.0f;

		sum = 0;
		count = 0;
		ReleaseSRWLockShared(&lock);
	}
};

extern MonitoringDataStats RTTData;