#include "core_io.h"

void FreeBufferC(BYTE *buffer)
{
    if (buffer != NULL)
    {
        free(buffer);
    }
}
// ============================================================================
// READ FILE BYTES
// ============================================================================
BYTE *ReadFileBytesC(const wchar_t *wPath, DWORD *outSize)
{
    // 1. Validar parámetros básicos de entrada
    if (IS_EMPTY(wPath) || !outSize)
        return NULL;
    *outSize = 0;

    // 2. Abrir el archivo de forma segura para lectura
    HANDLE hFile = CreateFileW(wPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return NULL;

    // 3. Obtener el tamaño del archivo de 64 bits (compatible con > 4GB)
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize))
    {
        CloseHandle(hFile);
        return NULL;
    }

    // Evitar desbordamiento si se intenta leer un archivo de más de 4GB en un DWORD
    if (fileSize.QuadPart > MAXDWORD)
    {
        CloseHandle(hFile);
        return NULL;
    }

    DWORD size = (DWORD)fileSize.QuadPart;

    // Manejo de archivos vacíos: devuelve un puntero válido no nulo, tamaño 0
    if (size == 0)
    {
        CloseHandle(hFile);
        return (BYTE *)malloc(1); // Retorna bloque mínimo para indicar éxito
    }

    // 4. Asignar memoria para el contenido
    BYTE *buffer = (BYTE *)malloc(size);
    if (!buffer)
    {
        CloseHandle(hFile);
        return NULL;
    }

    // 5. Leer el archivo de forma robusta
    DWORD bytesRead = 0;
    if (!ReadFile(hFile, buffer, size, &bytesRead, NULL))
    {
        // Si la API falla, liberamos la memoria inmediatamente
        free(buffer);
        buffer = NULL;
    }
    else
    {
        // Guardamos la cantidad real de bytes leídos
        *outSize = bytesRead;

        // Si se leyó menos de lo esperado, ajustamos la memoria (opcional)
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

    // 6. Limpieza de recursos
    CloseHandle(hFile);
    return buffer;
}

// ============================================================================
// WRITE FILE BYTES
// ============================================================================
BOOL WriteFileBytesC(const wchar_t *wPath, const BYTE *buffer, DWORD size)
{
    // 1. Validar parámetros básicos (Validación unificada con IS_EMPTY)
    if (IS_EMPTY(wPath))
        return FALSE;
    if (size > 0 && !buffer)
        return FALSE;

    // 2. Crear o sobrescribir el archivo de forma segura
    // CREATE_ALWAYS: Si el archivo ya existe, lo sobrescribe; si no, lo crea.
    HANDLE hFile = CreateFileW(wPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    // Si el tamaño es 0, simplemente creamos el archivo vacío y salimos con éxito
    if (size == 0)
    {
        CloseHandle(hFile);
        return TRUE;
    }

    // 3. Escribir los bytes en el archivo
    DWORD bytesWritten = 0;
    BOOL result = WriteFile(hFile, buffer, size, &bytesWritten, NULL);

    // 4. Verificar que se hayan escrito todos los bytes solicitados
    if (result && bytesWritten == size)
    {
        result = TRUE;
    }
    else
    {
        result = FALSE; // Falla si no se escribió todo (ej. disco lleno)
    }

    // 5. Cerrar el manejador del archivo
    CloseHandle(hFile);
    return result;
}
// ============================================================================
// 1. FILE LOGGING & AUTOMATIC ROTATION
// ============================================================================

// Appends raw bytes safely onto the disk
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
        return; // Si el archivo no existe aún, salimos normalmente para que AppendFileBytesC lo cree
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
        return; // CORRECCIÓN: return limpio para void

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
                // Copiar la ruta base quitando el ".log" final
                wcsncpy_s(wPathOld, MAX_PATH, wPath, longitud - 4);
                wPathOld[longitud - 4] = L'\0';

                // Concatenar el sufijo de respaldo
                wcscat_s(wPathOld, MAX_PATH, L"_OLD.log");

                // Intentar la rotación física en el disco
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
    
    // CORREGIDO: Añadido .%03d para formatear los milisegundos a 3 dígitos (Ej: .250)
    swprintf_s(logBuffer, 2048, L"[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\r\n", 
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, 
        st.wMilliseconds, // <-- Se inyecta el miembro de milisegundos aquí
        message); 
        
    DWORD sizeInBytes = (DWORD)(wcslen(logBuffer) * sizeof(wchar_t)); 
    return AppendFileBytesC(wPath, (const BYTE *)logBuffer, sizeInBytes); 
}

// ============================================================================
// 2. INI CONFIGURATION MANAGEMENT (WRITE, READ, DELETE)
// ============================================================================

// Writes a string value into an INI file
BOOL WriteIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, const wchar_t *value)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key) || IS_EMPTY(value))
        return FALSE;
    return WritePrivateProfileStringW(section, key, value, wPathIni);
}

// Reads a string value from an INI file
BOOL ReadIniString(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key,
                   const wchar_t *defaultValue, wchar_t *wResultBuffer, DWORD bufferSize)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key) || wResultBuffer == NULL || bufferSize == 0)
        return FALSE;

    // Note: defaultValue can be safely empty (L"") if specified by the developer
    DWORD result = GetPrivateProfileStringW(section, key, defaultValue, wResultBuffer, bufferSize, wPathIni);
    return (result > 0);
}

// Writes an integer value into an INI file
BOOL WriteIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int numberValue)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key))
        return FALSE;

    wchar_t numberBuffer[32];
    swprintf_s(numberBuffer, 32, L"%d", numberValue);
    return WritePrivateProfileStringW(section, key, numberBuffer, wPathIni);
}

// Reads an integer value directly from an INI file
int ReadIniInt(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key, int defaultValue)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key))
        return defaultValue;
    return GetPrivateProfileIntW(section, key, defaultValue, wPathIni);
}

// Deletes a single specific key under a section
BOOL DeleteIniKey(const wchar_t *wPathIni, const wchar_t *section, const wchar_t *key)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section) || IS_EMPTY(key))
        return FALSE;

    // Passing NULL as the value parameter deletes the key from the INI file
    return WritePrivateProfileStringW(section, key, NULL, wPathIni);
}

// Deletes an entire section along with all its keys
BOOL DeleteIniSection(const wchar_t *wPathIni, const wchar_t *section)
{
    if (IS_EMPTY(wPathIni) || IS_EMPTY(section))
        return FALSE;

    // Passing NULL both as the key and value parameters deletes the entire section
    return WritePrivateProfileStringW(section, NULL, NULL, wPathIni);
}

// ============================================================================
// 3. DYNAMIC PATH DISCOVERY
// ============================================================================

// Returns the path of the primary running .exe process
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

// Returns the path of your specific compiled DLL or FLL library file
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

// Returns a safe path in AppData\Roaming
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