#include "stdafx.h"

CTLSDBConnector dbConnector;

void DBWriteFunction(void)
{
    if (dbConnector.Query((WCHAR*)L"begin;") == FALSE)
    {
        wprintf_s(L"begin failed, error no : %d, error msg : %s\n", dbConnector.GetLastError(), dbConnector.GetLastErrorMessage());
    }

    if (dbConnector.Query((WCHAR*)L"UPDATE `character` SET `level` = `level` + 1 WHERE `character_no` = %lld;", 1) == FALSE)
    {
        wprintf_s(L"levelup failed, error no : %d, error msg : %s\n", dbConnector.GetLastError(), dbConnector.GetLastErrorMessage());
    }

    if (dbConnector.Query((WCHAR*)L"commit;") == FALSE)
    {
        wprintf_s(L"commit failed, error no : %d, error msg : %s\n", dbConnector.GetLastError(), dbConnector.GetLastErrorMessage());
    }

    if (dbConnector.Query((WCHAR*)L"SELECT * FROM `character`;") == FALSE)
    {
        wprintf_s(L"commit failed, error no : %d, error msg : %s\n", dbConnector.GetLastError(), dbConnector.GetLastErrorMessage());
    }

    MYSQL_ROW mysqlRow = dbConnector.FetchRow();

    dbConnector.FreeResult();

    printf_s("no : %s, level : %s, money : %s\n", mysqlRow[0], mysqlRow[1], mysqlRow[2]);

    return;
}


void GetTableName(WCHAR* pTableName, DWORD bufferCb);

DWORD WINAPI DBWriteThread(void* pParam)
{

RECONNECT:

    if (dbConnector.Connect((WCHAR*)L"procademyserver.iptime.org", 10950, (WCHAR*)L"accountdb", (WCHAR*)L"chanhun", (WCHAR*)L"Cksgns123$") == FALSE)
    {
        wprintf_s(L"connect failed, Error Code : %d, Error Msg : %s\n", dbConnector.GetLastError(), dbConnector.GetLastErrorMessage());

        Sleep(1000);

        goto RECONNECT;
    }

    if (dbConnector.Query((WCHAR*)L"SELECT userid,usernick FROM `account` where accountno = %d;", 1) == FALSE)
    {
        wprintf_s(L"Query Failed\n");
    }

    MYSQL_ROW mysqlRow = dbConnector.FetchRow();

    printf_s("userid : %s, usernick : %s", mysqlRow[0], mysqlRow[1]);

    dbConnector.FreeResult();

    dbConnector.Disconnect();

    return 1;
}



INT main()
{
    CTLSDBConnector DBConnector;

    if (DBConnector.Connect((WCHAR*)L"procademyserver.iptime.org", 10950, (WCHAR*)L"gamedb", (WCHAR*)L"chanhun", (WCHAR*)L"Cksgns123$") == FALSE)
    {
        wprintf_s(L"connect failed, Error Code : %d, Error Msg : %s\n", DBConnector.GetLastError(), DBConnector.GetLastErrorMessage());

        Sleep(1000);
    }

    if (DBConnector.Query((WCHAR*)L"SELECT * FROM accountdb.v_account where accountno = 1000000;") == TRUE)
    {
        wprintf_s(L"Success\n");
    }
    else
    {
        wprintf_s(L"Failed\n");
    }
    DBConnector.StoreResult();

    MYSQL_ROW mysqlRow = DBConnector.FetchRow();
    if (mysqlRow == nullptr)
    {
        std::wcout << L"mysqlRow is nullptr\n";

        return 1;
    }

    if (mysqlRow[3] == NULL)
    {
        std::wcout << L"index 3 is nullptr\n";
    }

    DBConnector.Disconnect();

    system("pause");

    return 1;
}

void GetTableName(WCHAR* pTableName, DWORD bufferCb)
{
    tm nowTime = { 0, };

    INT64 time64 = NULL;

    _time64(&time64);

    _localtime64_s(&nowTime, &time64);
 
    StringCbPrintf(pTableName, bufferCb, L"%d%02d_gameserver", nowTime.tm_year + 1900, nowTime.tm_mon + 1);

    return;
}
