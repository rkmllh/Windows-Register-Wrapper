#include "pch.h"
#include "framework.h"

#define _CRT_SECURE_NO_WARNINGS
#include "EditorRegister.h"

std::ostream& operator<<(std::ostream& os, _REG_VALUE rv)
{
	switch (rv.dwType)
	{

	case REG_NONE:
		os << "REG_NONE";
		break;

	case REG_SZ:
		os << "REG_SZ";
		break;

	case REG_EXPAND_SZ:
		os << "REG_EXPAND_SZ";
		break;

	case REG_BINARY:
		os << "REG_BINARY";
		break;

	case REG_DWORD_LITTLE_ENDIAN:
		os << "REG_DWORD_LITTLE_ENDIAN";
		break;

	case REG_DWORD_BIG_ENDIAN:
		os << "REG_DWORD_BIG_ENDIAN";
		break;

	case REG_LINK:
		os << "REG_LINK";
		break;

	case REG_MULTI_SZ:
		os << "REG_MULTI_SZ";
		break;

	case REG_RESOURCE_LIST:
		os << "REG_RESOURCE_LIST";
		break;

	case REG_FULL_RESOURCE_DESCRIPTOR:
		os << "REG_FULL_RESOURCE_DESCRIPTOR";
		break;

	case REG_RESOURCE_REQUIREMENTS_LIST:
		os << "REG_RESOURCE_REQUIREMENTS_LIST";
		break;

	case REG_QWORD_LITTLE_ENDIAN:
		os << "REG_QWORD_LITTLE_ENDIAN";
		break;

	default:
		os << "Unknown";
		break;
	}

	os << "\t\t'" << rv.ValueName << "'";

	switch (rv.dwType)
	{
	case REG_DWORD_LITTLE_ENDIAN:
	case REG_DWORD_BIG_ENDIAN:
	case REG_QWORD_LITTLE_ENDIAN:
	{
		DWORD val = 0;
		BYTE* pval = (BYTE*)&val;
		const BYTE* pdata = (BYTE*)rv.data;

		for (int i = 0; i < 4; ++i)
			*(pval)++ = *(pdata)++;

		os << "\t\t" << val << std::endl;
	}
	break;

	default:
		os << "\t\t" << rv.data << std::endl;
		break;
	}

	return os;
}

std::ostream& operator<<(std::ostream& os, _REG_KEY rk)
{
	SYSTEMTIME st;
	SecureZeroMemory(&st, sizeof(st));
	FileTimeToSystemTime(&rk.ftLastWriteTime, &st);
	os << st.wDay << "\\" << st.wMonth << "\\" << st.wYear << "\t";

	if (st.wHour < 10)
		os << "0";
	os << st.wHour << ":";

	if (st.wMinute < 10)
		os << "0";
	os << st.wMinute << ":";

	if (st.wSecond < 10)
		os << "0";
	os << st.wSecond << "\t";

	os << rk.KeyName << std::endl;
	return os;
}

LSTATUS _EDITOR_REGISTER::OpenKey(DWORD dwSam)
{
	this->Release();

	LSTATUS lsOpen = REG_NO_ERROR;

	lsOpen = RegOpenKeyEx(
		this->hKey,
		this->SubKey.c_str(),
		0,
		dwSam,
		&(this->phKey)
	);

	return lsOpen;
}

LSTATUS _EDITOR_REGISTER::CreateKey(DWORD dwOptions, PDWORD dwDisposition, DWORD dwSam)
{
	this->Release();

	LSTATUS lsCreate = REG_NO_ERROR;

	lsCreate = RegCreateKeyEx(
		this->hKey,
		this->SubKey.c_str(),
		0,
		NULL,
		dwOptions,
		KEY_ALL_ACCESS | dwSam,
		NULL,
		&(this->phKey),
		dwDisposition
	);

	return lsCreate;
}

LSTATUS _EDITOR_REGISTER::DeleteKey(DWORD dwSam)
{
	LSTATUS lsDelete = REG_NO_ERROR;

	lsDelete = RegDeleteKeyEx(
		this->hKey,
		this->SubKey.c_str(),
		dwSam,
		0
	);

	return lsDelete;
}

LSTATUS _EDITOR_REGISTER::DeleteValue()
{
	LSTATUS lsDelete = REG_NO_ERROR;

	lsDelete = RegDeleteValue(
		this->phKey,
		this->ValueName.c_str()
	);

	return lsDelete;
}

LSTATUS _EDITOR_REGISTER::SetValue(VOID* data, DWORD dwType, DWORD dwSize)
{
	LSTATUS lsSet = REG_NO_ERROR;

	lsSet = RegSetValueEx(
		this->phKey,
		this->ValueName.c_str(),
		0,
		dwType,
		(CONST BYTE*)data,
		dwSize
	);

	return lsSet;
}

LSTATUS _EDITOR_REGISTER::QueryValue(PREG_VALUE prv)
{
	if (!prv)
		return ERROR_NOACCESS;

	SecureZeroMemory(prv, sizeof(prv));

	LSTATUS lsQuery = REG_NO_ERROR;
	DWORD dwSize = 0;
	PBYTE pByte = NULL;

	lsQuery = RegQueryValueEx(
		this->phKey,
		this->ValueName.c_str(),
		NULL,
		&prv->dwType,
		NULL,
		&dwSize
	);

	if (lsQuery == ERROR_SUCCESS)
	{
		if (!(pByte = new BYTE[dwSize]))
			return REG_ERROR;

		lsQuery = RegQueryValueEx(
			this->phKey,
			this->ValueName.c_str(),
			NULL,
			&prv->dwType,
			pByte,
			&dwSize
		);

		if (lsQuery == ERROR_SUCCESS)
			strncpy((char*)prv->data, (const char*)pByte, MAX_PATH);

		strncpy((char*)prv->ValueName, (const char*)this->ValueName.c_str(), MAX_PATH);
		delete[]pByte;
	}

	return lsQuery;
}

LSTATUS _EDITOR_REGISTER::BrowseKey(std::vector<REG_KEY>* KeyVct, std::vector<REG_VALUE>* ValVct)
{
	if (!KeyVct && !ValVct)
		return ERROR_NOACCESS;

	LSTATUS lsEnum = ERROR_SUCCESS;
	char ClassKey[MAX_PATH];
	DWORD dwClassKey = MAX_PATH;
	DWORD dwSubKeys = 0;
	DWORD dwMaxSubKey = 0;
	DWORD dwMaxClass = 0;
	DWORD dwValues = 0;
	DWORD dwMaxValueName = 0;
	DWORD dwMaxValueData = 0;
	DWORD dwSecurityDescriptor = 0;
	FILETIME ftLastWriteTime;

	SecureZeroMemory(&ftLastWriteTime, sizeof(ftLastWriteTime));
	SecureZeroMemory(ClassKey, sizeof(ClassKey));

	lsEnum = RegQueryInfoKey(
		this->phKey,
		ClassKey,
		&dwClassKey,
		NULL,
		&dwSubKeys,
		&dwMaxSubKey,
		&dwMaxClass,
		&dwValues,
		&dwMaxValueName,
		&dwMaxValueData,
		&dwSecurityDescriptor,
		&ftLastWriteTime
	);

	for (unsigned int i = 0; i < dwSubKeys; ++i)
	{
		DWORD dwName = MAX_PATH;
		char Key[MAX_PATH];
		SecureZeroMemory(Key, MAX_PATH);

		lsEnum = RegEnumKeyEx(
			this->phKey,
			i,
			Key,
			&dwName,
			NULL,
			NULL,
			NULL,
			&ftLastWriteTime
		);

		if (lsEnum == ERROR_SUCCESS)
		{
			REG_KEY rk;
			SecureZeroMemory(&rk, sizeof(REG_KEY));

			strncpy(rk.KeyName, Key, MAX_PATH);
			memcpy(&rk.ftLastWriteTime, &ftLastWriteTime, sizeof(FILETIME));
			if(KeyVct)
				KeyVct->push_back(rk);
		}
	}

	for (unsigned int i = 0; i < dwValues; ++i)
	{
		DWORD szValue = MAX_PATH;
		char ValueName[MAX_PATH];
		char Data[MAX_PATH];
		DWORD szData = MAX_PATH;
		DWORD dwType = 0;

		SecureZeroMemory(ValueName, MAX_PATH);
		SecureZeroMemory(Data, MAX_PATH);

		lsEnum = RegEnumValue(
			this->phKey,
			i,
			ValueName,
			&szValue,
			NULL,
			&dwType,
			(LPBYTE)Data,
			&szData
		);

		if (lsEnum == ERROR_SUCCESS)
		{
			REG_VALUE rv;
			SecureZeroMemory(&rv, sizeof(REG_VALUE));

			rv.dwType = dwType;
			strncpy(rv.ValueName, ValueName, MAX_PATH);
			memcpy(rv.data, Data, MAX_PATH);

			if(ValVct)
				ValVct->push_back(rv);
		}
	}

	return lsEnum;
}

LSTATUS _EDITOR_REGISTER::GetValue(std::string SubKeyLocal, std::string ValueName, DWORD* pdwSize, DWORD dwType, void* data)
{
	return RegGetValueA(
		this->phKey,
		SubKeyLocal.c_str(),
		ValueName.c_str(),
		dwType,
		NULL,
		data,
		pdwSize
	);
}

LSTATUS _EDITOR_REGISTER::GetSize(PREG_SIZE prs)
{
	if (!prs)
		return REG_ERROR;

	PDH_STATUS pStatus = ERROR_SUCCESS;
	PDH_HQUERY hQuery = NULL;
	HCOUNTER hCounter = NULL;
	PDH_COUNTER_PATH_ELEMENTS_A pdhEl;
	PDH_RAW_COUNTER pdhRawCnt;
	char* CounterPath = NULL;
	DWORD dwCounterPath = PDH_MAX_COUNTER_PATH;
	DWORD dwType = 0;

	SecureZeroMemory(&pdhEl, sizeof(PDH_COUNTER_PATH_ELEMENTS_A));
	SecureZeroMemory(&pdhRawCnt, sizeof(PDH_RAW_COUNTER));

	__try
	{
		if (!(CounterPath = new char[PDH_MAX_COUNTER_PATH]))
		{
			pStatus = REG_ERROR;
			__leave;
		}

		SecureZeroMemory(CounterPath, PDH_MAX_COUNTER_PATH);

		pStatus = PdhOpenQuery(NULL, NULL, &hQuery);

		if (pStatus != ERROR_SUCCESS)
			__leave;

		pdhEl.szMachineName = NULL;
		pdhEl.szObjectName = (LPSTR)"System";
		pdhEl.szInstanceName = NULL;
		pdhEl.szParentInstance = NULL;
		pdhEl.dwInstanceIndex = 1;
		pdhEl.szCounterName = (LPSTR)"% Registry Quota In Use";

		pStatus = PdhMakeCounterPath(&pdhEl, CounterPath, &dwCounterPath, 0);

		if (pStatus != ERROR_SUCCESS)
			__leave;

		pStatus = PdhAddEnglishCounter(hQuery, CounterPath, 0, &hCounter);

		if (pStatus != ERROR_SUCCESS)
			__leave;

		pStatus = PdhCollectQueryData(hQuery);

		if (pStatus != ERROR_SUCCESS)
			__leave;

		pStatus = PdhGetRawCounterValue(hCounter, &dwType, &pdhRawCnt);

		if (pStatus != ERROR_SUCCESS)
			__leave;

		prs->dwCurrentSz = (DWORD)pdhRawCnt.FirstValue;
		prs->dwMaxSz = (DWORD)pdhRawCnt.SecondValue;
	}

	__finally
	{
		if (CounterPath)
			delete[]CounterPath;

		if (hQuery)
			PdhCloseQuery(hQuery);
	}

	return pStatus;
}

LSTATUS _EDITOR_REGISTER::RcQueryKey(DWORD dwSam, BOOL(*Callback)(std::vector<REG_KEY>&, std::vector<REG_VALUE>&))
{
	LSTATUS lsQuery = ERROR_SUCCESS;
	std::string lcSubKey;
	std::vector<REG_KEY> KeyVct;
	std::vector<REG_VALUE> ValVct;

	lcSubKey = { this->SubKey };
	this->BrowseKey(&KeyVct, &ValVct);

	if (Callback)
		if (!Callback(KeyVct, ValVct))
			return lsQuery;

	for (auto& v : KeyVct)
	{
		this->SetInternalSubKey(std::string{ lcSubKey + std::string{"\\"} + std::string{v.KeyName} });
		lsQuery = this->OpenKey(dwSam);

		if (lsQuery != ERROR_SUCCESS)
			return lsQuery;

		this->RcQueryKey(dwSam, Callback);
	}

	return lsQuery;
}

LSTATUS _EDITOR_REGISTER::SaveKey(std::string file, DWORD dwFlags)
{
	LSTATUS lsSave = ERROR_SUCCESS;

	lsSave = RegSaveKeyEx(
		this->phKey,
		file.c_str(),
		NULL,
		dwFlags
	);

	return lsSave;
}

LSTATUS _EDITOR_REGISTER::Flush()
{
	LSTATUS lsFlush = ERROR_SUCCESS;

	lsFlush = RegFlushKey(
		this->phKey
	);

	return lsFlush;
}

void _EDITOR_REGISTER::Release()
{
	if (this->phKey != NULL)
	{
		RegCloseKey(this->phKey);
		this->phKey = NULL;
	}
}

void _EDITOR_REGISTER::Clear()
{
	this->Release();
	this->hKey = NULL;
	this->SubKey = std::string{};
	this->ValueName = std::string{};
}

_EDITOR_REGISTER::~_EDITOR_REGISTER()
{
	this->Release();
}