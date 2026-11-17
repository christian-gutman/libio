#ifndef CORE_IO_H
#define CORE_IO_H

#include "core.h"
#include "core_string.h"

#include <shlwapi.h>
#include <shlobj.h> // Contiene SHGetKnownFolderPath y FOLDERID_RoamingAppData

#pragma comment(lib, "Shlwapi.lib")
 
#define LOG_MAX_SIZE (5 * 1024 * 1024)
 
#define IS_EMPTY(str) ((str) == NULL || (str)[0] == L'\0')

#ifdef __cplusplus
extern "C"
{
#endif
 
    
    CORE_API void FreeBufferC(BYTE *buffer);
 
    CORE_API BYTE *ReadFileBytesC(const wchar_t *wPath, DWORD *outSize);
 
    CORE_API BOOL WriteFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size);
 
    CORE_API BOOL AppendFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size);
 
    CORE_API void CheckAndRotateLog(const wchar_t *wPath);
 
    CORE_API BOOL WriteTextLogWithDate(const wchar_t *wPath, const wchar_t *message);
   
    //CORE_API BOOL WriteTextLogWithDate(const wchar_t *wPath, const wchar_t *message);
 
    CORE_API BOOL WriteIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, const wchar_t *value);
 
    CORE_API BOOL ReadIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key,
                       const wchar_t *defaultValue, wchar_t *wResultBuffer, DWORD bufferSize);
 
    CORE_API BOOL WriteIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int numberValue);
 
    CORE_API int ReadIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int defaultValue);
 
    CORE_API BOOL DeleteIniKey(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key);
 
    CORE_API BOOL DeleteIniSection(const wchar_t *wPathIni, const wchar_t *section);
 
    CORE_API BOOL GetPathInExeFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName);
 
    //CORE_API BOOL GetPathInLibraryFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName);
 
    CORE_API BOOL GetPathInLibraryFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName);

    CORE_API BOOL GetPathInAppData(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *appName, const wchar_t *logFileName);
/*
    BYTE *ReadFileBytesC(const wchar_t *wPath, DWORD *outSize);
    BOOL WriteFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size);

    BOOL AppendFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size);
    void ValidateOrRotateLog(const wchar_t *wPath);
    BOOL WriteTextLogWithDate(const wchar_t *wPath, const wchar_t *wMsg);

    BOOL GetPathAppData(wchar_t *wBufferPath, DWORD sizeBuffer, const wchar_t *appName, const wchar_t *fileNameLog);
    BOOL GetExecutionFolder(wchar_t *wBufferPath, DWORD sizeBuffer, const wchar_t *fileNameLog);
*/
static __forceinline void WriteStrLog(
        const char* hData,
        BOOL bSuccess,
        const char *Msg)
    {
        char szLogBuffer[512];
        DWORD dwErr = GetLastError();
        _snprintf_s(szLogBuffer, sizeof(szLogBuffer), _TRUNCATE,
                    "[LOG] %s | Success: %s | Data: %p | GetLastError: %lu",
                    Msg ? Msg : "Io Log Evt",
                    bSuccess ? "TRUE" : "FALSE",
                    hData,
                    dwErr);

        wchar_t wLogPath[MAX_PATH];
        if (GetPathInLibraryFolder(wLogPath, MAX_PATH, L"libio.log"))
        {
            wchar_t *wLogMessage = StrAutoToUnicode(szLogBuffer, (int)strlen(szLogBuffer), NULL);
            if (wLogMessage)
            {
                WriteTextLogWithDate(wLogPath, wLogMessage);
                free(wLogMessage);
            }
        }
    } 
 
#ifdef __cplusplus
}
#endif

#endif // CORE_IO_H
