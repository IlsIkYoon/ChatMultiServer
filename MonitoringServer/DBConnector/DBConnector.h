#pragma once

#include "Resource/MonitoringServerResource.h"

#pragma comment(lib, "mysqlclient.lib")
//#pragma comment(lib, "libmysql.lib")

#include "mysql.h"
#include "errmsg.h"



class CDBConnector
{
public:
	MYSQL conn;
	MYSQL* connection = NULL;
	MYSQL_RES* sql_result;
	MYSQL_ROW sql_row;
	int query_stat;


	bool DoQuery(const char* query)
	{
		query_stat = mysql_query(connection, query);
		if (query_stat != 0)
		{
			std::string error;
			error = mysql_error(&conn);
			printf("Mysql query error : %s", mysql_error(&conn));
			__debugbreak();
			return false;
		}
		return true;
	}

	CDBConnector()
	{
		mysql_init(&conn);

		//todo//read config 파일	
		// 멤버 변수에 DB ip 등의 값 저장
		//connection 바로 연결

		connection = mysql_real_connect(&conn, "127.0.0.1", "root", "12341234", "db_test", 3306, (char*)NULL, 0);

		if (connection == NULL)
		{
			// mysql_errno(&_MySQL);
			fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
			__debugbreak();
		}

		const char* createQuery =
			"CREATE TABLE IF NOT EXISTS MonitorServer_Data2 ("
			"timestamp VARCHAR(64), "
			"data_type VARCHAR(64), "
			"serverNo INT, "
			"avg_value INT, "
			"min_value INT, "
			"max_value INT"
			");";

		DoQuery(createQuery); // DoQuery는 실패 시 false 반환한다고 가정
	}

	~CDBConnector()
	{
		mysql_close(connection);
	}


	MYSQL_RES* StoreResult()
	{
		sql_result = mysql_store_result(connection);

		return sql_result;
	}

	bool FreeResult()
	{
		mysql_free_result(sql_result);
		return true;
	}
};

class CDBManager : public CDBConnector
{
public:

	struct MonitorData
	{
		std::string timeStamp;
		std::string dataType;
		int serverNo;
		int aver;
		int max;
		int min;
	};


	bool WriteMonitorData(MonitorData* data);

};
