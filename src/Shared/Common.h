#pragma once

#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <windows.h>

namespace Common {

// Text functions
bool WideToAnsi(const std::wstring &src, std::string &dest, UINT codePage = CP_ACP);
std::string WideToAnsi(const std::wstring &src, UINT codePage = CP_ACP);

bool AnsiToWide(const std::string &src, std::wstring &dest, UINT codePage = CP_ACP);
std::wstring AnsiToWide(const std::string &src, UINT codePage = CP_ACP);

std::string ToLower(const std::string &str);
std::wstring ToLower(const std::wstring &str);
std::string ToUpper(const std::string &str);
std::wstring ToUpper(const std::wstring &str);

void AddTrailingSlash(std::string &s);
void AddTrailingSlash(std::wstring &s);

std::wstring GetDirectoryName(const std::wstring &path);
std::string GetDirectoryName(const std::string &path);

// Registry fucntions
bool RegGetRawValue(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                    std::vector<char> &value, DWORD &type, DWORD flags);

bool RegSetRawValue(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                    const std::vector<char> &value, DWORD type, DWORD flags);

std::wstring RegGetString(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                          const std::wstring &defaultValue = L"", DWORD flags = 0);

bool RegSetString(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                  const std::wstring &value, DWORD flags = 0);

std::vector<std::wstring> RegGetMultiString(HKEY hRoot, const std::wstring &key,
                                            const std::wstring &name, DWORD flags = 0);

bool RegSetMultiString(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                       const std::vector<std::wstring> &value, DWORD flags = 0);

DWORD RegGetDword(HKEY hRoot, const std::wstring &key, const std::wstring &name, DWORD defaultValue,
                  DWORD flags = 0);

bool RegSetDword(HKEY hRoot, const std::wstring &key, const std::wstring &name, DWORD value,
                 DWORD flags = 0);

static inline bool RegGetBool(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                              bool defaultValue = false, DWORD flags = 0)
{
    return RegGetDword(hRoot, key, name, defaultValue ? 1 : 0, flags) != 0;
}

static inline bool RegSetBool(HKEY hRoot, const std::wstring &key, const std::wstring &name,
                              bool value, DWORD flags = 0)
{
    return RegSetDword(hRoot, key, name, value ? 1 : 0, flags);
}

// File functions
bool ReadFileFull(const std::wstring &filename, std::string &result);
bool ReadFileFull(const std::wstring &filename, std::vector<char> &result);
bool WriteFileFull(const std::wstring &filename, const void *data, size_t size);
bool WriteFileFull(const std::wstring &filename, const std::string &value);
bool WriteFileFull(const std::wstring &filename, const std::vector<char> &value);

static inline bool FileExists(const std::wstring &filename)
{
    DWORD attrs = GetFileAttributesW(filename.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static inline bool FileExists(const std::string &filename)
{
    DWORD attrs = GetFileAttributesA(filename.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static inline bool DirExists(const std::wstring &filename)
{
    DWORD attrs = GetFileAttributesW(filename.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

static inline bool DirExists(const std::string &filename)
{
    DWORD attrs = GetFileAttributesA(filename.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring GetUniqPath(std::wstring dir, const std::wstring &prefix = L"",
                         const std::wstring &suffix = L"");

static inline std::string GetUniqPath(const std::string &dir, const std::string &prefix = "",
                                      const std::string &suffix = "")
{
    return WideToAnsi(GetUniqPath(AnsiToWide(dir), AnsiToWide(prefix), AnsiToWide(suffix)));
}

bool GetFullPath(const std::string &path, std::string &result);
bool GetFullPath(const std::wstring &path, std::wstring &result);

// Other functions
void ErrorMsgBox(UINT exitCode, const char *format, ...);
std::wstring GetCurrentModulePath();
uint32_t GetCrc32(const void *data, size_t size, uint32_t crc = 0);

// Just an assert which will always compile (not debug only)
#define VERIFY(cond) \
    if (!(cond)) {   \
        abort();     \
    }

} // namespace Common
