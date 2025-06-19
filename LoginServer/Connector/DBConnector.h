#pragma once

#include "Resource/LoginServerResource.h"

//#pragma comment(lib, "mysqlclient.lib")
#pragma comment(lib, "libmysql.lib")

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



	CDBConnector()
	{
		mysql_init(&conn);

		//todo//read config 파일
		// 멤버 변수에 DB ip 등의 값 저장
		//connection 바로 연결

		connection = mysql_real_connect(&conn, "127.0.0.1", "root", "12341234", "test", 3306, (char*)NULL, 0);

		if (connection == NULL)
		{
			// mysql_errno(&_MySQL);
			fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
			__debugbreak();
		}
	}

	~CDBConnector()
	{
		mysql_close(connection);
	}

	bool DoQuery(char* query)
	{
		query_stat = mysql_query(connection, query);
		if (query_stat != 0)
		{
			printf("Mysql query error : %s", mysql_error(&conn));
			__debugbreak();
			return false;
		}
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

/*
--------------------------------------------------------------------------


C Connector 에서  Mysql 8.0 연결시 아래 쿼리를 먼저 반영하여
패스워드 방식을 예전 방식으로 변경 해야 합니다.

ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'password';


// 결과출력
sql_result = mysql_store_result(connection);		// 결과 전체를 미리 가져옴
//	sql_result=mysql_use_result(connection);		// fetch_row 호출시 1개씩 가져옴

while ((sql_row = mysql_fetch_row(sql_result)) != NULL)
{
	printf("%2s %2s %s\n", sql_row[0], sql_row[1], sql_row[2]);
}
mysql_free_result(sql_result);

// DB 연결닫기
mysql_close(connection);




//	int a = mysql_insert_id(connection);


//	query_stat = mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);
//	mysql_next_result(connection);				// 멀티쿼리 사용시 다음 결과 얻기
//	sql_result=mysql_store_result(connection);	// next_result 후 결과 얻음

*/


class CDBManager : public CDBConnector 
{


public:

	bool LoginDataRequest(CPacket* message, ULONG64 characterKey);


};


