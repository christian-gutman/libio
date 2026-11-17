#include "core_io.h"

void FreeBufferC(BYTE *buffer)
{
    if (buffer != NULL)
    {
        free(buffer);
    }
} 

BYTE *ReadFileBytesC(const wchar_t *wPath, DWORD *outSize)
{ 
    if (IS_EMPTY(wPath) || !outSize)
        return NULL;
    *outSize = 0;
 
    HANDLE hFile = CreateFileW(wPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;
 
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        CloseHandle(hFile);
        return NULL;
    }
 
    if (fileSize.QuadPart > MAXDWORD)
    {
        CloseHandle(hFile);
        return NULL;
    }

    DWORD size = (DWORD)fileSize.QuadPart;
 
    if (size == 0)
    {
        CloseHandle(hFile);
        return (BYTE *)malloc(1); 
    }
 
    BYTE *buffer = (BYTE *)malloc(size);
    if (!buffer)
    {
        CloseHandle(hFile);
        return NULL;
    }
 
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, size, &bytesRead, NULL))
    { 
        free(buffer);
        buffer = NULL;
    }
    else
    { 
        *outSize = bytesRead;
 
        if (bytesRead < size && bytesRead > 0)
        {
            BYTE *temp = (BYTE *)realloc(buffer, bytesRead);
            if (temp)
                buffer = temp;
        }
        else if (bytesRead == 0)
        {
            free(buffer);
            buffer = NULL;
        }
    }
 
    CloseHandle(hFile);
    return buffer;
}
 
BOOL WriteFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size)
{
 
    if (IS_EMPTY(wPath))
        return FALSE;
    if (size > 0 && !buffer)
        return FALSE;

 
    HANDLE hFile = CreateFileW(wPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;
 
    if (size == 0)
    {
        CloseHandle(hFile);
        return TRUE;
    }
 
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(hFile, buffer, size, &bytesWritten, NULL);
 
    if (result && bytesWritten == size)
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;  
    }
 
    CloseHandle(hFile);
    return result;
} 

BOOL AppendFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size)
{
    if (IS_EMPTY(wPath))
        return FALSE;
    if (size > 0 && !buffer)
        return FALSE;

    HANDLE hFile = CreateFileW(wPath, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    if (size == 0)
    {
        CloseHandle(hFile);
        return TRUE;
    }

    SetFilePointer(hFile, 0, NULL, FILE_END);
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(hFile, buffer, size, &bytesWritten, NULL);
    if (result && bytesWritten == size)
    {
        result = TRUE;
    }
    else
    {
        result = FALSE;
    }

    CloseHandle(hFile);
    return result;
}
void CheckAndRotateLog(const wchar_t *wPath)
{
    if (!wPath || wPath[0] == L'\0')
        return;

    HANDLE hFile = CreateFileW(
        wPath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return; 
    }

    LARGE_INTEGER tamanoArchivo;
    if (GetFileSizeEx(hFile, &tamanoArchivo))
    {
        CloseHandle(hFile);

        if (tamanoArchivo.QuadPart > LOG_MAX_SIZE)
        {
            wchar_t wPathOld[MAX_PATH];
            size_t longitud = wcslen(wPath);

            if (longitud > 4 && longitud < MAX_PATH)
            {
                wcsncpy_s(wPathOld, MAX_PATH, wPath, longitud - 4);
                wPathOld[longitud - 4] = L'\0';
                wcscat_s(wPathOld, MAX_PATH, L"_OLD.log");

                DeleteFileW(wPathOld);
                MoveFileW(wPath, wPathOld);
            }
        }
    }
    else
    {
        CloseHandle(hFile);
    }
}
void CheckAndRotateLogOLD(const wchar_t *wPath)
{
    if (!wPath || wPath[0] == L'\0')
        return;  

    HANDLE hFile = NULL;
    hFile = CreateFileW(hFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    LARGE_INTEGER tamanoArchivo;
    if (GetFileSizeEx(hFile, &tamanoArchivo))
    {
        CloseHandle(hFile);

        if (tamanoArchivo.QuadPart > LOG_MAX_SIZE)
        {
            wchar_t wPathOld[MAX_PATH];
            size_t longitud = wcslen(wPath);

            if (longitud > 4 && longitud < MAX_PATH)
            { 
                wcsncpy_s(wPathOld, MAX_PATH, wPath, longitud - 4);
                wPathOld[longitud - 4] = L'\0';
 
                wcscat_s(wPathOld, MAX_PATH, L"_OLD.log");
 
                DeleteFileW(wPathOld);
                MoveFileW(wPath, wPathOld);
            }
        }
    }
    else
    {
        CloseHandle(hFile);
    }
}

BOOL WriteTextLogWithDate(const wchar_t *wPath, const wchar_t *message) { 
    if (IS_EMPTY(wPath) || IS_EMPTY(message)) return FALSE; 
    
    CheckAndRotateLog(wPath); 
    
    SYSTEMTIME st; 
    GetLocalTime(&st); 
    
    wchar_t logBuffer[2048]; 
     
    swprintf_s(logBuffer, 2048, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\r\n", 
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, 
        st.wMilliseconds, 
        message); 
        
    DWORD sizeInBytes = (DWORD)(wcslen(logBuffer) * sizeof(wchar_t)); 
    return AppendFileBytesC(wPath, (const BYTE *)logBuffer, sizeInBytes); 
}
 
BOOL WriteIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, const wchar_t *value)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key) || IS_EMPTY(value))
        return FALSE;
    return WritePrivateProfileStringW(section, key, value, wPathIni);
}
 
BOOL ReadIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key,
                   const wchar_t *defaultValue, wchar_t *wResultBuffer, DWORD bufferSize)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key) || wResultBuffer == NULL || bufferSize == 0)
        return FALSE;
 
    DWORD result = GetPrivateProfileStringW(section, key, defaultValue, wResultBuffer, bufferSize, wPathIni);
    return (result > 0);
}
 
BOOL WriteIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int numberValue)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key))
        return FALSE;

    wchar_t numberBuffer[32];
    swprintf_s(numberBuffer, 32, L"%d", numberValue);
    return WritePrivateProfileStringW(section, key, numberBuffer, wPathIni);
}
 
int ReadIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int defaultValue)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key))
        return defaultValue;
    return GetPrivateProfileIntW(section, key, defaultValue, wPathIni);
}
 
BOOL DeleteIniKey(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key))
        return FALSE;
 
    return WritePrivateProfileStringW(section, key, NULL, wPathIni);
}
 
BOOL DeleteIniSection(const wchar_t *wPathIni, const wchar_t *section)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section))
        return FALSE;
 
    return WritePrivateProfileStringW(section, NULL, NULL, wPathIni);
} 

BOOL GetPathInExeFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName)
{
    if (wPathBuffer == NULL || bufferSize == 0 || IS_EMPTY(logFileName))
        return FALSE;

    DWORD result = GetModuleFileNameW(NULL, wPathBuffer, bufferSize);
    if (result == 0 || result >= bufferSize)
        return FALSE;

    wchar_t *lastSlash = wcsrchr(wPathBuffer, L'\\');
    if (lastSlash != NULL)
    {
        *(lastSlash + 1) = L'\0';
        wcscat_s(wPathBuffer, bufferSize, logFileName);
        return TRUE;
    }
    return FALSE;
}
 
BOOL GetPathInLibraryFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName)
{
    if (wPathBuffer == NULL || bufferSize == 0 || IS_EMPTY(logFileName))
        return FALSE;

    HINSTANCE hCurrentLibrary = NULL;
    BOOL hasHandle = GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)GetPathInLibraryFolder,
        &hCurrentLibrary);
    if (!hasHandle || hCurrentLibrary == NULL)
        return FALSE;

    DWORD result = GetModuleFileNameW(hCurrentLibrary, wPathBuffer, bufferSize);
    if (result == 0 || result >= bufferSize)
        return FALSE;

    wchar_t *lastSlash = wcsrchr(wPathBuffer, L'\\');
    if (lastSlash != NULL)
    {
        *(lastSlash + 1) = L'\0';
        wcscat_s(wPathBuffer, bufferSize, logFileName);
        return TRUE;
    }
    return FALSE;
}
 
BOOL GetPathInAppData(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *appName, const wchar_t *logFileName)
{
    if (wPathBuffer == NULL || bufferSize == 0 || IS_EMPTY(appName) || IS_EMPTY(logFileName))
        return FALSE;

    PWSTR pAppDataPath = NULL;
    HRESULT hr = SHGetKnownFolderPath(&FOLDERID_RoamingAppData, KF_FLAG_CREATE, NULL, &pAppDataPath);
    if (hr != S_OK || pAppDataPath == NULL)
        return FALSE;

    wchar_t appFolder[MAX_PATH];
    swprintf_s(appFolder, MAX_PATH, L"%s\\%s", pAppDataPath, appName);
    CreateDirectoryW(appFolder, NULL);

    swprintf_s(wPathBuffer, bufferSize, L"%s\\%s", appFolder, logFileName);
    CoTaskMemFree(pAppDataPath);
    return TRUE;
}