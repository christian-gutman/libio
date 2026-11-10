#ifndef CORE_IO_H
#define CORE_IO_H

#include "core.h"
#include "core_string.h"

#include <shlwapi.h>
#include <shlobj.h> // Contiene SHGetKnownFolderPath y FOLDERID_RoamingAppData

#pragma comment(lib, "Shlwapi.lib")

// Max log size constraint before rotation triggers (5 MB)
#define LOG_MAX_SIZE (5 * 1024 * 1024)

// Helper macro to validate if a wide string pointer is null or completely empty
#define IS_EMPTY(str) ((str) == NULL || (str)[0] == L'\0')

#ifdef __cplusplus
extern "C"
{
#endif

    // ============================================================================
    // 0. RAW FILE BYTES OPERATIONS
    // ============================================================================
    // Safely frees heap memory allocated by ReadFileBytesC
    CORE_API void FreeBufferC(BYTE *buffer);
    // Reads all bytes from a file. Memory must be freed using free() by the caller.
    CORE_API BYTE *ReadFileBytesC(const wchar_t *wPath, DWORD *outSize);

    // Creates or overwrites a file with the provided byte buffer
    CORE_API BOOL WriteFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size);

    // ============================================================================
    // 1. FILE LOGGING & AUTOMATIC ROTATION
    // ============================================================================

    // Appends raw bytes safely onto the disk
    CORE_API BOOL AppendFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size);

    // Renames the log file to _OLD if it exceeds LOG_MAX_SIZE
    CORE_API void CheckAndRotateLog(const wchar_t *wPath);

    // Main high-level wrapper to log strings with timestamps
    CORE_API BOOL WriteTextLogWithDate(const wchar_t *wPath, const wchar_t *message);

    // ============================================================================
    // 2. INI CONFIGURATION MANAGEMENT (WRITE, READ, DELETE)
    // ============================================================================

    // Writes a string value into an INI file
    CORE_API BOOL WriteIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, const wchar_t *value);

    // Reads a string value from an INI file
    CORE_API BOOL ReadIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key,
                       const wchar_t *defaultValue, wchar_t *wResultBuffer, DWORD bufferSize);

    // Writes an integer value into an INI file
    CORE_API BOOL WriteIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int numberValue);

    // Reads an integer value directly from an INI file
    CORE_API int ReadIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int defaultValue);

    // Deletes a single specific key under a section
    CORE_API BOOL DeleteIniKey(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key);

    // Deletes an entire section along with all its keys
    CORE_API BOOL DeleteIniSection(const wchar_t *wPathIni, const wchar_t *section);

    // ============================================================================
    // 3. DYNAMIC PATH DISCOVERY
    // ============================================================================

    // Returns the path of the primary running .exe process
    CORE_API BOOL GetPathInExeFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName);

    // Returns the path of your specific compiled DLL or FLL library file
    CORE_API BOOL GetPathInLibraryFolder(wchar_t *wPathBuffer, DWORD bufferSize, const wchar_t *logFileName);

    // Returns a safe path in AppData\Roaming
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
 
#ifdef __cplusplus
}
#endif

#endif // CORE_IO_H
