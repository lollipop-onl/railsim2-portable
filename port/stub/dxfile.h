#pragma once

#include "windows.h"

struct IDirectXFile;
struct IDirectXFileEnumObject;
struct IDirectXFileData;

typedef IDirectXFile* LPDIRECTXFILE;
typedef IDirectXFileEnumObject* LPDIRECTXFILEENUMOBJECT;
typedef IDirectXFileData* LPDIRECTXFILEDATA;

#define DXFILE_OK 0

#define DXFILELOAD_FROMFILE 0x00L
#define DXFILELOAD_FROMRESOURCE 0x01L

struct DXFILELOADRESOURCE {
  HMODULE hModule;
  LPCSTR lpName;
  LPCSTR lpType;
};

struct IDirectXFile : IUnknown {
  HRESULT CreateEnumObject(void*, DWORD, IDirectXFileEnumObject**) { return S_OK; }
  HRESULT RegisterTemplates(void*, DWORD) { return S_OK; }
};

struct IDirectXFileEnumObject : IUnknown {
  HRESULT GetNextDataObject(IDirectXFileData**) { return S_OK; }
};

struct IDirectXFileData : IUnknown {
  HRESULT GetName(const char**, DWORD*) { return S_OK; }
  HRESULT GetData(const GUID*, DWORD*, void**) { return S_OK; }
  HRESULT GetType(const GUID**) { return S_OK; }
};

inline HRESULT DirectXFileCreate(IDirectXFile**) { return E_NOTIMPL; }
