#pragma once

#include "CDBConnector.h"

namespace tlsdbconnector
{
	constexpr DWORD MAX_QUERY_LENGTH = 2000;
};


class CTLSDBConnector
{
public:

	CTLSDBConnector(void)
		:mTLSIndex(0)
	{
		mTLSIndex = TlsAlloc();
		if (mTLSIndex == TLS_OUT_OF_INDEXES)
		{
			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"TLSDBConnector", L"[CTLSDBConnector] Error Code : %d", GetLastError());

			CCrashDump::Crash();
		}
	}

	~CTLSDBConnector(void)
	{
		if (TlsFree(mTLSIndex) == FALSE)
		{
			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"TLSDBConnector", L"[~CTLSDBConnector] Error Code : %d", GetLastError());

			CCrashDump::Crash();
		}
	}

	BOOL Connect(WCHAR* pConnectIP, DWORD connectPort, WCHAR* pSchema, WCHAR* pUserID, WCHAR* pUserPassword)
	{
		CDBConnector* pDBConnector = getDBConnector();

		if (pDBConnector->Connect(pConnectIP, connectPort, pSchema, pUserID, pUserPassword) == FALSE)
		{
			return FALSE;
		}

		return TRUE;
	}

	BOOL Reconnect(void)
	{
		CDBConnector* pDBConnector = getDBConnector();

		if (pDBConnector->Reconnect() == FALSE)
		{
			return FALSE;
		}

		return TRUE;
	}

	BOOL Disconnect(void)
	{
		BOOL retval;

		CDBConnector* pDBConnector = getDBConnector();

		retval = pDBConnector->Disconnect();

		delete pDBConnector;

		if (TlsSetValue(mTLSIndex, nullptr) == 0)
		{
			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"TLSDBConnector", L"[Disconnect] Error Code : %d", GetLastError());

			CCrashDump::Crash();
		}

		return retval;
	}

	BOOL MySQLCharacterSet(WCHAR* pCharacterSet)
	{
		CDBConnector* pDBConnector = getDBConnector();

		if (pDBConnector->MySQLCharacterSet(pCharacterSet) == FALSE)
		{
			return FALSE;
		}

		return TRUE;
	}


	BOOL Query(WCHAR* pQueryFormat, ...)
	{
		CDBConnector* pDBConnector = getDBConnector();

		WCHAR wideByteQuery[tlsdbconnector::MAX_QUERY_LENGTH];

		va_list va = NULL;

		va_start(va, pQueryFormat);

		HRESULT retval = NULL;

		retval = StringCchVPrintfW(wideByteQuery, tlsdbconnector::MAX_QUERY_LENGTH, pQueryFormat, va);
		if (FAILED(retval))
		{
			CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"DBConnector", L"[Query] Error Code : %d, return : %d, format : %s, va : %s", GetLastError(), retval, pQueryFormat, va);

			CCrashDump::Crash();
		}

		if (pDBConnector->Query(wideByteQuery) == FALSE)
		{
			return FALSE;
		}

		return TRUE;
	}

	void StoreResult(void)
	{
		getDBConnector()->StoreResult();

		return;
	}

	MYSQL_ROW FetchRow(void)
	{		
		return getDBConnector()->FetchRow();
	}

	void FreeResult(void)
	{		
		getDBConnector()->FreeResult();

		return;
	}

	BOOL GetConnectStateFlag(void)
	{
		
		return getDBConnector()->GetConnectStateFlag();
	}

	INT GetLastError(void)
	{
		
		return getDBConnector()->GetLastError();
	}

	const WCHAR* GetLastErrorMessage(void)
	{
		
		return getDBConnector()->GetLastErrorMessage();
	}

	BOOL CheckReconnectErrorCode(void)
	{
		return getDBConnector()->CheckReconnectErrorCode();
	}

private:

	CDBConnector* getDBConnector(void)
	{
		CDBConnector* pDBConnector = (CDBConnector*)TlsGetValue(mTLSIndex);
		if (pDBConnector == nullptr)
		{
			pDBConnector = new CDBConnector;

			if (TlsSetValue(mTLSIndex, (PVOID)pDBConnector) == 0)
			{
				CSystemLog::GetInstance()->Log(TRUE, CSystemLog::eLogLevel::LogLevelError, L"TLSDBConnector", L"[setDBConnector] Error Code : %d", GetLastError());

				CCrashDump::Crash();
			}
		}

		return pDBConnector;
	}

	DWORD mTLSIndex;
};

