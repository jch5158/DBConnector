#pragma once
#include <iostream>
#include <Windows.h>
#include <strsafe.h>
#include "SystemLogLibrary/SystemLogLibrary/CSystemLog.h"
#include "mysql/include/mysql.h"
#include "mysql/include/errmsg.h"

#pragma comment(lib,"mysql/lib/vs14/mysqlclient.lib")

namespace dbconnector
{	
	constexpr DWORD MAX_QUERY_LENGTH = 2000;

	constexpr DWORD MAX_IP_LENGTH = 50;

	constexpr DWORD MAX_ID_LENGTH = 30;

	constexpr DWORD MAX_PASSWORD_LENGTH = 100;

	constexpr DWORD MAX_ERROR_MESSAGE_LENGTH = 300;

	constexpr DWORD MAX_SCHEMA_LENGTH = 100;
};


// DB 연결 및 쿼리 전송 그리고 예외처리 정도만 하는 클래스이다. 
class CDBConnector
{
public:

	CDBConnector(void)
		: mMySQL{ 0, }
		, mpMySQL(nullptr)
		, mpMySQLResult(nullptr)
		, mbConnectStateFlag(FALSE)
		, mLastErrorCode(0)
		, mConnectPort(0)
		, mConnectIP{ 0, }
		, mSchema{ 0, }
		, mUserID{ 0, }
		, mUserPassword{ 0, }
		, mLastErrorMessage{ 0, }
		, mWideByteQuery{ 0, }
		, mMultiByteQuery{ 0, }
	{
		mysql_init(&mMySQL);

		// MYSQL ping 호출 시 재연결 설정
		bool reconnect = 1; // 1;

		mysql_options(&mMySQL, MYSQL_OPT_RECONNECT, &reconnect);
	}


	~CDBConnector(void)
	{
	}

	BOOL Connect(WCHAR* pConnectIP, DWORD connectPort, WCHAR* pSchema, WCHAR* pUserID, WCHAR* pUserPassword)
	{	
		setConnectInfo(pConnectIP, connectPort, pSchema, pUserID, pUserPassword);

		CHAR connectIP[dbconnector::MAX_IP_LENGTH];

		WideCharToMultiByte(CP_ACP, 0, mConnectIP, -1, connectIP, dbconnector::MAX_IP_LENGTH, NULL, NULL);

		CHAR schema[dbconnector::MAX_SCHEMA_LENGTH];

		WideCharToMultiByte(CP_ACP, 0, mSchema, -1, schema, dbconnector::MAX_SCHEMA_LENGTH, NULL, NULL);

		CHAR userID[dbconnector::MAX_ID_LENGTH];

		WideCharToMultiByte(CP_ACP, 0, mUserID, -1, userID, dbconnector::MAX_ID_LENGTH, NULL, NULL);

		CHAR userPassword[dbconnector::MAX_PASSWORD_LENGTH];

		WideCharToMultiByte(CP_ACP, 0, mUserPassword, -1, userPassword, dbconnector::MAX_PASSWORD_LENGTH, NULL, NULL);

		mpMySQL = mysql_real_connect(&mMySQL, connectIP, userID, userPassword, schema, connectPort, nullptr, 0);
		if (mpMySQL == nullptr)
		{
			mLastErrorCode = mysql_errno(&mMySQL);

			setWideByteErrorMessage(mysql_error(&mMySQL));

			return FALSE;
		}

		mbConnectStateFlag = TRUE;

		return TRUE;
	}

	BOOL Reconnect(void)
	{
		if (CheckReconnectErrorCode() == TRUE)
		{
			if (mysql_ping(mpMySQL) != 0)
			{
				mLastErrorCode = mysql_errno(mpMySQL);

				setWideByteErrorMessage(mysql_error(mpMySQL));

				return FALSE;
			}
		}
		else
		{
			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"[DBConnector]", L"[Reconnect] LastErrorCode : %d, Error Message : %s", mLastErrorCode, mLastErrorMessage);

			CCrashDump::Crash();
		}

		return TRUE;
	}

	BOOL Disconnect(void)
	{
		if (mpMySQL == nullptr)
		{
			return FALSE;
		}

		mysql_close(mpMySQL);

		mbConnectStateFlag = FALSE;

		mpMySQL = nullptr;

		return TRUE;
	}


	BOOL MySQLCharacterSet(WCHAR* pCharacterSet)
	{
		CHAR multiByteBuffer[MAX_PATH];

		WideCharToMultiByte(CP_ACP, 0, pCharacterSet, -1, multiByteBuffer, MAX_PATH, NULL, NULL);
		if (mysql_set_character_set(mpMySQL, multiByteBuffer) != 0)
		{
			mLastErrorCode = mysql_errno(mpMySQL);

			setWideByteErrorMessage(mysql_error(&mMySQL));

			return FALSE;
		}

		return TRUE;
	}


	BOOL Query(WCHAR* pQueryFormat, ...)
	{
		va_list va = NULL;

		va_start(va, pQueryFormat);

		HRESULT retval = NULL;

		retval = StringCchVPrintfW(mWideByteQuery, dbconnector::MAX_QUERY_LENGTH, pQueryFormat, va);
		if (FAILED(retval))
		{
			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"DBConnector", L"[Query] Error Code : %d, return : %d, format : %s, va : %s", GetLastError(), retval, pQueryFormat, va);

			CCrashDump::Crash();
		}

		WideCharToMultiByte(CP_ACP, 0, mWideByteQuery, -1, mMultiByteQuery, dbconnector::MAX_QUERY_LENGTH, NULL, NULL);

		if (mysql_query(mpMySQL, mMultiByteQuery) != 0)
		{		
			mLastErrorCode = mysql_errno(mpMySQL);

			setWideByteErrorMessage(mysql_error(&mMySQL));

			return FALSE;
		}

		return TRUE;
	}

	void StoreResult(void)
	{
		// 결과 전체를 미리 가져옴
		mpMySQLResult = mysql_store_result(mpMySQL);
		
		return;
	}
	
	
	MYSQL_ROW FetchRow(void)
	{
		return mysql_fetch_row(mpMySQLResult);
	}


	void FreeResult(void)
	{
		mysql_free_result(mpMySQLResult);

		return;
	}

	BOOL GetConnectStateFlag(void) const
	{
		return mbConnectStateFlag;
	}

	INT GetLastError(void) const
	{
		return mLastErrorCode;
	}


	const WCHAR* GetLastErrorMessage(void) const
	{
		return (const WCHAR*)mLastErrorMessage;
	}


	BOOL CheckReconnectErrorCode(void)
	{
		switch (mLastErrorCode)
		{
		case CR_SERVER_GONE_ERROR:

			break;
		case CR_SERVER_LOST:

			break;
		case CR_CONN_HOST_ERROR:

			break;
		case CR_SERVER_HANDSHAKE_ERR:

			break;
		default:
			return FALSE;
		}

		return TRUE;
	}

private:

	void setConnectInfo(WCHAR* pConnectIP, DWORD connectPort, WCHAR* pSchema, WCHAR* pUserID, WCHAR* pUserPassword)
	{
		wcscpy_s(mConnectIP, pConnectIP);

		mConnectPort = connectPort;

		wcscpy_s(mSchema, pSchema);

		wcscpy_s(mUserID, pUserID);

		wcscpy_s(mUserPassword, pUserPassword);

		return;
	}


	void setWideByteErrorMessage(const CHAR* errorMessage)
	{
		MultiByteToWideChar(CP_ACP, 0, errorMessage, -1, mLastErrorMessage, MAX_PATH);

		return;
	}
	


	MYSQL mMySQL;

	MYSQL* mpMySQL;

	MYSQL_RES* mpMySQLResult;

	BOOL mbConnectStateFlag;

	INT mLastErrorCode;

	DWORD mConnectPort;

	WCHAR mConnectIP[dbconnector::MAX_IP_LENGTH];

	WCHAR mSchema[dbconnector::MAX_SCHEMA_LENGTH];

	WCHAR mUserID[dbconnector::MAX_ID_LENGTH];

	WCHAR mUserPassword[dbconnector::MAX_PASSWORD_LENGTH];

	WCHAR mLastErrorMessage[dbconnector::MAX_ERROR_MESSAGE_LENGTH];

	WCHAR mWideByteQuery[dbconnector::MAX_QUERY_LENGTH];

	CHAR mMultiByteQuery[dbconnector::MAX_QUERY_LENGTH];

};


