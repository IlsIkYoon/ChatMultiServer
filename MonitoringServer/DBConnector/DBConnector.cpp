#include "DBConnector.h"



bool CDBManager::WriteMonitorData(MonitorData* data)
{
	std::string query =
		"INSERT INTO MonitorServer_Data2 (timestamp, data_type, serverNo, avg_value, min_value, max_value) "
		"VALUES ('" + data->timeStamp + "', '" + data->dataType + "', '" + std::to_string(data->serverNo) + 
		"', '" + std::to_string(data->aver) + "', " + std::to_string(data->min) + ", " + std::to_string(data->max) + ");";

	DoQuery(query.c_str());

	return true;
}