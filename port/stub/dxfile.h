#pragma once

#include "windows.h"

struct IDirectXFile;
struct IDirectXFileEnumObject;
struct IDirectXFileData;

typedef IDirectXFile* LPDIRECTXFILE;
typedef IDirectXFileEnumObject* LPDIRECTXFILEENUMOBJECT;
typedef IDirectXFileData* LPDIRECTXFILEDATA;

struct IDirectXFile : IUnknown {
  HRESULT CreateEnumObject(void*, DWORD, IDirectXFileEnumObject**) { return S_OK; }
};

struct IDirectXFileEnumObject : IUnknown {
  HRESULT GetNextDataObject(IDirectXFileData**) { return S_OK; }
};

struct IDirectXFileData : IUnknown {
  HRESULT GetName(const char**, DWORD*) { return S_OK; }
  HRESULT GetData(const GUID*, DWORD*, void**) { return S_OK; }
};

inline HRESULT DirectXFileCreate(GUID*, IDirectXFile**) { return E_NOTIMPL; }
