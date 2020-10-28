#include "Common.h"
#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <stdexcept>

using namespace std;

static void RemoveTrailingNullChar(wstring &s)
{
    if (!s.empty() && s[s.length() - 1] == L'\0')
        s.resize(s.size() - 1);
}

static void RemoveTrailingNullChar(string &s)
{
    if (!s.empty() && s[s.length() - 1] == '\0')
        s.resize(s.size() - 1);
}

bool Common::WideToAnsi(const wstring &src, string &dest, UINT codePage)
{
    int lengthDest = WideCharToMultiByte(codePage, 0, src.c_str(), (int)src.length(), NULL, 0, NULL,
                                         NULL);
    if (lengthDest <= 0)
        return false;

    vector<char> buf(lengthDest + 1, '\0');
    lengthDest = WideCharToMultiByte(codePage, 0, src.c_str(), (int)src.length(), buf.data(),
                                     lengthDest, NULL, NULL);
    if (lengthDest <= 0)
        return false;

    dest.assign(buf.data(), lengthDest);
    RemoveTrailingNullChar(dest);
    return true;
}

string Common::WideToAnsi(const wstring &src, UINT codePage)
{
    string result;
    WideToAnsi(src, result, codePage);
    return result;
}

bool Common::AnsiToWide(const string &src, wstring &dest, UINT codePage)
{
    int lengthDest = MultiByteToWideChar(codePage, 0, src.c_str(), (int)src.length(), NULL, 0);
    if (lengthDest <= 0)
        return false;

    vector<wchar_t> buf(lengthDest + 1, L'\0');
    lengthDest = MultiByteToWideChar(codePage, 0, src.c_str(), (int)src.length(), buf.data(),
                                     lengthDest);
    if (lengthDest <= 0)
        return false;

    dest.assign(buf.data(), lengthDest);
    RemoveTrailingNullChar(dest);
    return true;
}

wstring Common::AnsiToWide(const string &src, UINT codePage)
{
    wstring result;
    AnsiToWide(src, result, codePage);
    return result;
}

string Common::ToLower(const string &str)
{
    vector<char> buf(str.length() + 1);
    strcpy(&buf[0], str.c_str());
    CharLowerA(&buf[0]);
    return string(buf.begin(), buf.end() - 1);
}

wstring Common::ToLower(const wstring &str)
{
    vector<wchar_t> buf(str.length() + 1);
    wcscpy(&buf[0], str.c_str());
    CharLowerW(&buf[0]);
    return wstring(buf.begin(), buf.end() - 1);
}

string Common::ToUpper(const string &str)
{
    vector<char> buf(str.length() + 1);
    strcpy(&buf[0], str.c_str());
    CharUpperA(&buf[0]);
    return string(buf.begin(), buf.end() - 1);
}

wstring Common::ToUpper(const wstring &str)
{
    vector<wchar_t> buf(str.length() + 1);
    wcscpy(&buf[0], str.c_str());
    CharUpperW(&buf[0]);
    return wstring(buf.begin(), buf.end() - 1);
}

void Common::AddTrailingSlash(string &s)
{
    if (!s.empty() && s[s.length() - 1] != '\\')
        s += '\\';
}

void Common::AddTrailingSlash(wstring &s)
{
    if (!s.empty() && s[s.length() - 1] != L'\\')
        s += L'\\';
}

wstring Common::GetDirectoryName(const wstring &path)
{
    if (path.empty() || path[path.length() - 1] == L'\\')
        return path;
    auto pos = path.rfind(L'\\');
    if (pos == path.npos)
        return wstring();
    wstring result = path;
    result.replace(result.begin() + pos + 1, result.end(), wstring());
    return result;
}

string Common::GetDirectoryName(const string &path)
{
    if (path.empty() || path[path.length() - 1] == '\\')
        return path;
    auto pos = path.rfind('\\');
    if (pos == path.npos)
        return string();
    string result = path;
    result.replace(result.begin() + pos + 1, result.end(), string());
    return result;
}

bool Common::RegGetRawValue(HKEY hRoot, const wstring &key, const wstring &name,
                            vector<char> &value, DWORD &type, DWORD flags)
{
    HKEY hKey;
    LSTATUS status = RegOpenKeyExW(hRoot, key.c_str(), 0, KEY_READ | flags, &hKey);
    if (status != ERROR_SUCCESS)
        return false;

    bool result = false;
    do
    {
        DWORD size = 0;
        status = RegQueryValueExW(hKey, name.c_str(), NULL, &type, NULL, &size);
        if (status != ERROR_SUCCESS || size == 0)
            break;

        value.resize(size);
        status = RegQueryValueExW(hKey, name.c_str(), NULL, &type, (BYTE *)&value[0], &size);
        if (status != ERROR_SUCCESS)
            break;

        value.resize(size);
        result = true;
    } while (false);

    RegCloseKey(hKey);
    return result;
}

bool Common::RegSetRawValue(HKEY hRoot, const wstring &key, const wstring &name,
                            const vector<char> &value, DWORD type, DWORD flags)
{
    HKEY hKey;
    LSTATUS status = RegCreateKeyExW(hRoot, key.c_str(), 0, NULL, REG_OPTION_VOLATILE,
                                     KEY_WRITE | flags, NULL, &hKey, NULL);
    if (status != ERROR_SUCCESS)
        return false;

    status = RegSetValueExW(hKey, name.c_str(), 0, type,
                            value.empty() ? NULL : (const BYTE *)&value[0], value.size());

    RegCloseKey(hKey);
    if (status != ERROR_SUCCESS)
        return false;

    return true;
}

wstring Common::RegGetString(HKEY hRoot, const wstring &key, const wstring &name,
                             const wstring &defaultValue, DWORD flags)
{
    vector<char> data;
    DWORD type;
    if (!RegGetRawValue(hRoot, key, name, data, type, flags) ||
        !(type == REG_SZ || type == REG_EXPAND_SZ))
        return defaultValue;

    wstring result((const wchar_t *)&data[0], data.size() / sizeof(wchar_t));
    RemoveTrailingNullChar(result);
    return result;
}

bool Common::RegSetString(HKEY hRoot, const wstring &key, const wstring &name, const wstring &value,
                          DWORD flags)
{
    vector<char> data;
    data.resize((value.length() + 1) * sizeof(value[0]));
    memcpy(&data[0], value.c_str(), data.size());
    return RegSetRawValue(hRoot, key, name, data, REG_SZ, flags);
}

vector<wstring> Common::RegGetMultiString(HKEY hRoot, const wstring &key, const wstring &name,
                                          DWORD flags)
{
    vector<char> data;
    DWORD type;
    if (!RegGetRawValue(hRoot, key, name, data, type, flags) || type != REG_MULTI_SZ)
        return vector<wstring>();

    vector<wstring> result;
    WCHAR *buf = (WCHAR *)&data[0];
    size_t size = data.size() / sizeof(WCHAR);
    for (size_t i = 0; i < size; i++)
    {
        size_t start = i;
        while (i < size && buf[i] != L'\0')
            i++;
        if (i > start)
            result.push_back(wstring(buf + start, i - start));
    }
    return result;
}

bool Common::RegSetMultiString(HKEY hRoot, const wstring &key, const wstring &name,
                               const vector<wstring> &value, DWORD flags)
{
    vector<char> data;
    size_t size = 0;
    for (auto it = value.cbegin(); it != value.cend(); ++it)
        size += (it->length() + 1) * sizeof((*it)[0]);
    data.resize(size);
    size = 0;
    for (auto it = value.cbegin(); it != value.cend(); ++it)
    {
        size_t elemSize = (it->length() + 1) * sizeof((*it)[0]);
        memcpy(&data[size], it->c_str(), elemSize);
        size += elemSize;
    }
    return RegSetRawValue(hRoot, key, name, data, REG_MULTI_SZ, flags);
}

DWORD Common::RegGetDword(HKEY hRoot, const wstring &key, const wstring &name, DWORD defaultValue,
                          DWORD flags)
{
    vector<char> data;
    DWORD type, result = defaultValue;
    if (RegGetRawValue(hRoot, key, name, data, type, flags) && type == REG_DWORD &&
        data.size() == sizeof(DWORD))
    {
        memcpy(&result, &data[0], sizeof(result));
    }
    return result;
}

bool Common::RegSetDword(HKEY hRoot, const wstring &key, const wstring &name, DWORD value,
                         DWORD flags)
{
    vector<char> data;
    data.resize(sizeof(value));
    memcpy(&data[0], &value, data.size());
    return RegSetRawValue(hRoot, key, name, data, REG_DWORD, flags);
}

bool Common::ReadFileFull(const wstring &filename, vector<char> &result)
{
    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    LARGE_INTEGER largeSize;
    if (!GetFileSizeEx(hFile, &largeSize) || largeSize.QuadPart > MAXDWORD)
    {
        CloseHandle(hFile);
        return false;
    }

    size_t size = (DWORD)largeSize.QuadPart;
    result.resize(size);

    DWORD bytesRead;
    BOOL res = ReadFile(hFile, &result[0], size, &bytesRead, NULL);
    CloseHandle(hFile);

    if (!res || bytesRead != size)
        return false;

    return true;
}

bool Common::ReadFileFull(const wstring &filename, string &result)
{
    vector<char> data;
    bool res = ReadFileFull(filename, data);
    result.assign(data.begin(), data.end());
    return res;
}

bool Common::WriteFileFull(const wstring &filename, const void *data, size_t size)
{
    HANDLE hFile = CreateFileW(filename.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return false;

    DWORD bytesWritten;
    BOOL res = WriteFile(hFile, data, size, &bytesWritten, NULL);
    CloseHandle(hFile);

    if (!res || bytesWritten != size)
        return false;

    return true;
}

bool Common::WriteFileFull(const wstring &filename, const vector<char> &value)
{
    return WriteFileFull(filename, &value[0], value.size());
}

bool Common::WriteFileFull(const wstring &filename, const string &value)
{
    return WriteFileFull(filename, value.c_str(), value.length());
}

static string GetFileDescription(const void *block)
{
    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    } * translate;
    UINT iLen;

    if (!VerQueryValueA(block, "\\VarFileInfo\\Translation", (LPVOID *)&translate, &iLen))
        return string();

    char *buffer = NULL;
    UINT bufLen = 0;
    for (size_t i = 0; i < (iLen / sizeof(struct LANGANDCODEPAGE)); i++)
    {
        char subBlock[100];
        sprintf_s(subBlock, "\\StringFileInfo\\%04x%04x\\FileDescription", translate[i].wLanguage,
                  translate[i].wCodePage);

        if (VerQueryValueA(block, subBlock, (void **)&buffer, &bufLen))
            break;
    }
    if (buffer == NULL || bufLen == 0)
        return string();

    string result(buffer, buffer + bufLen);
    RemoveTrailingNullChar(result);
    return result;
}

static string GetFileVersionString(const void *block)
{
    VS_FIXEDFILEINFO *info;
    UINT len;
    if (!VerQueryValueA(block, "\\", (void **)&info, &len))
        return string();

    char buf[100];
    sprintf_s(buf, "v%d.%d.%d.%d", HIWORD(info->dwFileVersionMS), LOWORD(info->dwFileVersionMS),
              HIWORD(info->dwFileVersionLS), LOWORD(info->dwFileVersionLS));
    return string(buf);
}

static string GetAppName(HMODULE hModule)
{
    char path[MAX_PATH] = "\0";
    if (!GetModuleFileNameA(hModule, path, sizeof(path)))
        return string();

    DWORD temp;
    DWORD len = GetFileVersionInfoSizeA(path, &temp);
    if (len == 0)
        return string();

    vector<BYTE> block(len);
    if (!GetFileVersionInfoA(path, NULL, len, &block[0]))
        return string();

    string appName = GetFileDescription(&block[0]);
    string ver = GetFileVersionString(&block[0]);
    if (!ver.empty())
        appName += " " + ver;
    return appName;
}

static HMODULE GetCurrentModule()
{
    HMODULE hModule;
    BOOL succ = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                       GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                   __FUNCTION__, &hModule);
    return succ ? hModule : NULL;
}

static string GetCurrentAppName()
{
    HMODULE hModule = GetCurrentModule();
    return hModule != NULL ? GetAppName(hModule) : string();
}

wstring Common::GetCurrentModulePath()
{
    HMODULE hModule = GetCurrentModule();
    if (hModule == NULL)
        return wstring();

    WCHAR buf[MAX_PATH];
    if (GetModuleFileNameW(hModule, buf, _countof(buf)) == 0)
        return wstring();

    return wstring(buf);
}

wstring Common::GetUniqPath(wstring dir, const wstring &prefix, const wstring &suffix)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    AddTrailingSlash(dir);
    for (int i = 0; i < 1000; i++)
    {
        WCHAR name[100];
        swprintf_s(name, L"%s%02d.%02d.%d-%d%s", prefix.c_str(), st.wDay, st.wMonth, st.wYear, i,
                   suffix.c_str());
        wstring result = dir + name;
        if (GetFileAttributesW(result.c_str()) == INVALID_FILE_ATTRIBUTES)
            return result;
    }
    return wstring();
}

bool Common::GetFullPath(const std::string &path, std::string &result)
{
    vector<char> buffer(16, L'\0');
    size_t size = GetFullPathNameA(path.c_str(), buffer.size(), &buffer[0], nullptr);
    if (size > buffer.size())
    {
        buffer.resize(size);
        size = GetFullPathNameA(path.c_str(), buffer.size(), &buffer[0], nullptr);
    }
    if (size > 0)
    {
        assert(size <= buffer.size());
        result.assign(&buffer[0]);
        return true;
    }
    return false;
}

bool Common::GetFullPath(const std::wstring &path, std::wstring &result)
{
    vector<wchar_t> buffer(16, L'\0');
    size_t size = GetFullPathNameW(path.c_str(), buffer.size(), &buffer[0], nullptr);
    if (size > buffer.size())
    {
        buffer.resize(size);
        size = GetFullPathNameW(path.c_str(), buffer.size(), &buffer[0], nullptr);
    }
    if (size > 0)
    {
        assert(size <= buffer.size());
        result.assign(&buffer[0]);
        return true;
    }
    return false;
}

void Common::ErrorMsgBox(UINT exitCode, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), format, args);
    MessageBoxA(NULL, buf, GetCurrentAppName().c_str(), MB_OK | MB_ICONERROR);
    va_end(args);
    if (exitCode != 0)
        ExitProcess(exitCode);
}

static uint32_t Crc32ForByte(uint32_t b)
{
    for (int i = 0; i < 8; i++)
        b = (b & 1 ? 0 : (uint32_t)0xEDB88320ul) ^ b >> 1;
    return b ^ (uint32_t)0xFF000000ul;
}

uint32_t Common::GetCrc32(const void *data, size_t size, uint32_t crc)
{
    static uint32_t table[0x100];
    if (!*table)
    {
        for (size_t i = 0; i < 256; i++)
            table[i] = Crc32ForByte(i);
    }

    for (size_t i = 0; i < size; i++)
        crc = table[(uint8_t)crc ^ ((uint8_t *)data)[i]] ^ crc >> 8;
    return crc;
}
