#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <vector>
#include <pdh.h>

#pragma comment (lib,"pdh.lib")

typedef struct _REG_VALUE
{
	DWORD dwType;
	CHAR ValueName[MAX_PATH];
	BYTE data[MAX_PATH];
}REG_VALUE, * PREG_VALUE;

typedef struct _REG_KEY
{
	FILETIME ftLastWriteTime;
	CHAR KeyName[MAX_PATH];
}REG_KEY, * PREG_KEY;

typedef struct _REG_SIZE
{
	DWORD dwMaxSz;
	DWORD dwCurrentSz;
}REG_SIZE, * PREG_SIZE;

std::ostream& operator<<(
	std::ostream& os,
	_REG_VALUE rv
);

std::ostream& operator<<(
	std::ostream& os,
	_REG_KEY rk
);

class _EDITOR_REGISTER
{

public:

	_EDITOR_REGISTER() :
		SubKey{ std::string{} },
		ValueName{ std::string{} },
		phKey{ NULL },
		hKey{ NULL }{}

	_EDITOR_REGISTER(std::string SKey) :
		SubKey{ SKey },
		ValueName{ std::string{} },
		phKey{ NULL },
		hKey{ NULL }{}

	_EDITOR_REGISTER(std::string SKey, std::string Value) :
		SubKey{ SKey },
		ValueName{ Value },
		phKey{ NULL },
		hKey{ NULL }{}

	_EDITOR_REGISTER(std::string SKey, std::string Value, HKEY hk) :
		SubKey{ SKey },
		ValueName{ Value },
		phKey{ NULL },
		hKey{ hk }{}

	_EDITOR_REGISTER(std::string SKey, std::string Value, HKEY phK, HKEY hk) :
		SubKey{ SKey },
		ValueName{ Value },
		phKey{ phK },
		hKey{ hk }{}

	virtual LSTATUS OpenKey(DWORD dwSam);
	virtual LSTATUS CreateKey(DWORD dwOptions, PDWORD dwDisposition, DWORD dwSam);
	virtual LSTATUS DeleteKey(DWORD dwSam);
	virtual LSTATUS DeleteValue();

	LSTATUS SetValue(VOID* data, DWORD dwType, DWORD dwSize);

	virtual LSTATUS QueryValue(PREG_VALUE prv);
	virtual LSTATUS BrowseKey(std::vector<REG_KEY>* KeyVct, std::vector<REG_VALUE>* ValVct);
	virtual LSTATUS GetValue(std::string SubKeyLocal, std::string ValueName, DWORD* pdwSize, DWORD dwType, void* data);

	virtual LSTATUS GetSize(PREG_SIZE prs);
	virtual LSTATUS RcQueryKey(DWORD dwSam, BOOL(*Callback)(std::vector<REG_KEY>& KeyVct, std::vector<REG_VALUE>& ValVct));
	virtual LSTATUS SaveKey(std::string file, DWORD dwFlags);
	virtual LSTATUS Flush();

	void SetInternalSubKey(const std::string& sk) { this->SubKey = sk; }
	void SetInternalValueName(const std::string& nv) { this->ValueName = nv; }
	void SetInternalHKey(const HKEY hk) { this->hKey = hk; }
	void SetInternalPHKey(const HKEY pHk) { this->phKey = pHk; }

	std::string GetSubKey(void)		const { return this->SubKey; }
	std::string GetNameKey(void)		const { return this->ValueName; }
	HKEY GetHKey(void)		const { return this->hKey; }
	HKEY GetPHKey(void)		const { return this->phKey; }

	virtual void Release();
	virtual void Clear();

	virtual ~_EDITOR_REGISTER();

private:

	HKEY phKey;
	HKEY hKey;
	std::string ValueName;
	std::string SubKey;
};

using EDITOR_REGISTER = _EDITOR_REGISTER;
using MODIFIER_REGISTER = _EDITOR_REGISTER;
using PEDITOR_REGISTER = _EDITOR_REGISTER*;
using PMODIFIER_REGISTER = _EDITOR_REGISTER*;
using PREGISTER_HANDLE = _EDITOR_REGISTER*;

/*
	Register status
*/

#define REG_NO_ERROR				NO_ERROR
#define REG_ERROR					~REG_NO_ERROR