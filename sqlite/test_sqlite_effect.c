#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/time.h>
#include"sqlite3.h"

int main(int argc, char* argv[])
{
	int rc = 0;
	int i = 0;
	int j = 0;
	int rows, cols;
	int n1, n2;
	sqlite3* db = NULL;
	sqlite3* dbMem = NULL;
	char* zErr = NULL;
	char **pRecord = NULL;
	sqlite3_stmt *stmt = NULL;
	char *buf = "CJcEEAAYASCgExEFAaATEqATEy";
	struct timeval  tmv1;
	struct timeval  tmv2;
	float tmcost;

	char tmpstr[32] = {0};
	char sqlcmd[2048] = {0};
	const char* ch1;
	const char* ch2;
	
	if(argc != 2){
	    fprintf(stderr, "%% ERROR! Usage: %s <operation_num>\n", argv[0]);  
	    return 1;  
    }  

	long long int data_num = atoll(argv[1]);
	if(data_num <= 0){
		fprintf(stderr, "%% ERROR! Usage: %s <operation_num>\n", argv[0]);  
	    return 1;
	}

	rc = sqlite3_open("kaf.db", &db);
	if (rc)
	{
		fprintf(stderr, "Can't open database:%s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}
	
	rc = sqlite3_exec(db, "create table if not exists testinfo (id integer primary key, age integer, height text, weight text)", NULL, NULL, &zErr);
	if (SQLITE_OK != rc) {
        fprintf(stderr, "create sql failed:%s\n", zErr);
        sqlite3_close(db);
        exit(1);
    }
	
	//sqlite3_exec()逐条插入
	if (atoi(argv[1]) == 1)
	{
		gettimeofday(&tmv1, NULL);
		for( i = 0; i < 1000000; i++)
		{
			snprintf(sqlcmd, sizeof(sqlcmd), "insert into testinfo values(%d, %d, '%d', '%s')", i, i*2, i*10, buf);
			sqlite3_exec(db, sqlcmd, NULL, NULL, &zErr);
		}
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 1 operation costs %f\n", tmcost);
	}
	//开启事务
	else if(atoi(argv[1]) == 2)
	{
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(db, "BEGIN;", 0, 0, NULL);
		for( i = 0; i < 1000000; i++)
		{
			snprintf(sqlcmd, sizeof(sqlcmd), "insert into testinfo values(%d, %d, '%d', '%s')", i, i*2, i*10, buf);
			sqlite3_exec(db, sqlcmd, NULL, NULL, &zErr);
		}
		sqlite3_exec(db, "COMMIT;", 0, 0, NULL);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 2 operation costs %f\n", tmcost);
	}
	//执行准备
	else if(atoi(argv[1]) == 3)
	{
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(db, "BEGIN;", 0, 0, 0);
		const char* sql = "insert into testinfo values(?,?,?,?)";
		sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sprintf(tmpstr, "%d", i*10);
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			sqlite3_bind_int(stmt, 2, i*2);
			sqlite3_bind_text(stmt, 3, tmpstr, -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 4, buf, -1, SQLITE_STATIC);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(db, "COMMIT;", 0, 0, 0);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 4 operation costs %f\n", tmcost);
	}
	//关闭写同步
	else if(atoi(argv[1]) == 4)
	{
		sqlite3_exec(db, "PRAGMA synchronous = OFF; ", 0,0,0);  
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(db, "BEGIN;", 0, 0, 0);
		const char* sql = "insert into testinfo values(?,?,?,?)";
		sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sprintf(tmpstr, "%d", i*10);
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			sqlite3_bind_int(stmt, 2, i*2);
			sqlite3_bind_text(stmt, 3, tmpstr, -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 4, buf, -1, SQLITE_STATIC);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(db, "COMMIT;", 0, 0, 0);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 5 operation costs %f\n", tmcost);
	}
	//使用WAL模式
	else if(atoi(argv[1]) == 5)
	{
		sqlite3_exec(db, "PRAGMA journal_mode=WAL; ", 0,0,0);  
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(db, "BEGIN;", 0, 0, 0);
		const char* sql = "insert into testinfo values(?,?,?,?)";
		sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sprintf(tmpstr, "%d", i*10);
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			sqlite3_bind_int(stmt, 2, i*2);
			sqlite3_bind_text(stmt, 3, tmpstr, -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 4, buf, -1, SQLITE_STATIC);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(db, "COMMIT;", 0, 0, 0);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 5 operation costs %f\n", tmcost);
	}
	//内存数据库
	else if(atoi(argv[1]) == 6)
	{	
		rc = sqlite3_open(":memory:", &dbMem);
		rc = sqlite3_exec(dbMem, "create table if not exists testinfo (id integer primary key, age integer, height text, weight text)", NULL, NULL, &zErr);
		sqlite3_exec(dbMem, "PRAGMA synchronous = OFF; ", 0,0,0);  
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(dbMem, "BEGIN;", 0, 0, 0);
		const char* sql = "insert into testinfo values(?,?,?,?)";
		sqlite3_prepare_v2(dbMem, sql, strlen(sql), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sprintf(tmpstr, "%d", i*10);
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			sqlite3_bind_int(stmt, 2, i*2);
			sqlite3_bind_text(stmt, 3, tmpstr, -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 4, buf, -1, SQLITE_STATIC);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(dbMem, "COMMIT;", 0, 0, 0);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 6 operation costs %f\n", tmcost);
	}
	//sqlite3_get_table()逐条查询
	else if(atoi(argv[1]) == 7)
	{
		gettimeofday(&tmv1, NULL);
		for( i = 0; i < 1000000; i++)
		{
			snprintf(sqlcmd, sizeof(sqlcmd), "select * from testinfo where id = %d", i);
			sqlite3_get_table(db, sqlcmd, &pRecord, &rows, &cols, &zErr);
		}
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 7 operation costs %f\n", tmcost);
	}
	//开启事务
	else if(atoi(argv[1]) == 8)
	{
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(db, "BEGIN", 0, 0, NULL);
		for( i = 0; i < 1000000; i++)
		{
			snprintf(sqlcmd, sizeof(sqlcmd), "select * from testinfo where id = %d", i);
			sqlite3_get_table(db, sqlcmd, &pRecord, &rows, &cols, &zErr);
		}
		sqlite3_exec(db, "COMMIT", 0, 0, NULL);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 8 operation costs %f\n", tmcost);
	}
	//执行准备
	else if(atoi(argv[1]) == 9)
	{
		char *sql = "select * from testinfo where id = ?";
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(db, "BEGIN", 0, 0, NULL);
		sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW)
			{
				n1  = sqlite3_column_int(stmt, 0);
				n2  = sqlite3_column_int(stmt, 1);
				ch1 = sqlite3_column_text(stmt, 2);
				ch2 = sqlite3_column_text(stmt, 3);
				rc = rc = sqlite3_step(stmt);
			}
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(db, "COMMIT", 0, 0, NULL);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 9 operation costs %f\n", tmcost);
	}
	//内存数据库
	else if(atoi(argv[1]) == 10)
	{
		rc = sqlite3_open(":memory:", &dbMem);
		rc = sqlite3_exec(dbMem, "create table if not exists testinfo (id integer primary key, age integer, height text, weight text)", NULL, NULL, &zErr);
		sqlite3_exec(dbMem, "PRAGMA synchronous = OFF; ", 0,0,0);  
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(dbMem, "BEGIN;", 0, 0, 0);
		const char* sql = "insert into testinfo values(?,?,?,?)";
		sqlite3_prepare_v2(dbMem, sql, strlen(sql), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sprintf(tmpstr, "%d", i*10);
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			sqlite3_bind_int(stmt, 2, i*2);
			sqlite3_bind_text(stmt, 3, tmpstr, -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 4, buf, -1, SQLITE_STATIC);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(dbMem, "COMMIT;", 0, 0, 0);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 6 operation costs %f\n", tmcost);
		
		
		char *sql2 = "select * from testinfo where id = ?";
		gettimeofday(&tmv1, NULL);
		sqlite3_exec(dbMem, "BEGIN", 0, 0, NULL);
		sqlite3_prepare_v2(dbMem, sql2, strlen(sql2), &stmt, 0);
		for(i = 0; i < 1000000; i++)
		{
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, i);
			rc = sqlite3_step(stmt);
			while(rc == SQLITE_ROW)
			{
				n1  = sqlite3_column_int(stmt, 0);
				n2  = sqlite3_column_int(stmt, 1);
				ch1 = sqlite3_column_text(stmt, 2);
				ch2 = sqlite3_column_text(stmt, 3);
				rc = rc = sqlite3_step(stmt);
			}
		}
		sqlite3_finalize(stmt);
		sqlite3_exec(dbMem, "COMMIT", 0, 0, NULL);
		gettimeofday(&tmv2, NULL);
		tmcost = (float)(tmv2.tv_sec*1000*1000+tmv2.tv_usec - tmv1.tv_sec*1000*1000+tmv1.tv_usec)/1000000;
		printf("the 9 operation costs %f\n", tmcost);
	}
	
	sqlite3_close(db);
	return 1;
}

