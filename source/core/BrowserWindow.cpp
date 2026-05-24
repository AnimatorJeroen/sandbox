#include "pch.h"
#include "BrowserWindow.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>
#include <sstream>
#include <algorithm>
#include "Logger.h"

namespace Core
{
    WindowsBrowserWindow::WindowsBrowserWindow(void* windowHandle)
        : m_windowHandle(windowHandle)
    {
        LOG_DEBUG() << "WindowsBrowserWindow initialized with handle: " << windowHandle;
    }

    std::string WindowsBrowserWindow::BuildFilterString(const std::vector<FileFilter>& filters) const
    {
        if (filters.empty())
        {
            // Default: All Files
            return std::string("All Files\0*.*\0\0", 17); // Explicitly include null terminators
        }

        std::string result;
        for (const auto& filter : filters)
        {
            result += filter.description;
            result += '\0';
            result += filter.pattern;
            result += '\0';
        }
        result += '\0'; // Double null terminator

        return result;
    }

    std::optional<std::string> WindowsBrowserWindow::OpenFile(
        const std::string& title,
        const std::vector<FileFilter>& filters,
        const std::string& defaultPath)
    {
        LOG_DEBUG() << "OpenFile called: title=" << title << ", windowHandle=" << m_windowHandle;
        
        char filename[MAX_PATH] = { 0 };
        
        if (!defaultPath.empty())
        {
            strncpy_s(filename, MAX_PATH, defaultPath.c_str(), MAX_PATH - 1);
        }

        // Keep filter string alive for the duration of the dialog
        std::string filterStr = BuildFilterString(filters);

        OPENFILENAMEA ofn = { 0 };
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = static_cast<HWND>(m_windowHandle); // Can be NULL for no parent
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = filterStr.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        LOG_DEBUG() << "Calling GetOpenFileNameA...";
        BOOL result = GetOpenFileNameA(&ofn);
        
        if (result)
        {
            LOG_DEBUG() << "File selected: " << filename;
            return std::string(filename);
        }
        else
        {
            DWORD error = CommDlgExtendedError();
            if (error == 0)
            {
                LOG_DEBUG() << "User canceled file dialog";
            }
            else
            {
                LOG_ERROR() << "GetOpenFileNameA failed with CommDlgExtendedError: 0x" << std::hex << error << std::dec;
                LOG_ERROR() << "GetLastError: " << GetLastError();
                
                // Log more details about the failure
                if (error == 0xFFFF || error == 65535)
                {
                    LOG_ERROR() << "Error 0xFFFF indicates dialog initialization failure";
                    LOG_ERROR() << "Common causes: invalid window handle, wrong thread, or resource issues";
                }
            }
        }

        return std::nullopt;
    }

    std::vector<std::string> WindowsBrowserWindow::OpenFiles(
        const std::string& title,
        const std::vector<FileFilter>& filters,
        const std::string& defaultPath)
    {
        LOG_DEBUG() << "OpenFiles called: title=" << title;
        
        // Buffer for multiple file selection (need larger buffer)
        constexpr size_t BUFFER_SIZE = 65536;
        std::vector<char> filename(BUFFER_SIZE, 0);

        if (!defaultPath.empty())
        {
            strncpy_s(filename.data(), BUFFER_SIZE, defaultPath.c_str(), BUFFER_SIZE - 1);
        }

        // Keep filter string alive for the duration of the dialog
        std::string filterStr = BuildFilterString(filters);

        OPENFILENAMEA ofn = { 0 };
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = static_cast<HWND>(m_windowHandle);
        ofn.lpstrFile = filename.data();
        ofn.nMaxFile = static_cast<DWORD>(BUFFER_SIZE);
        ofn.lpstrFilter = filterStr.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER | OFN_NOCHANGEDIR;

        std::vector<std::string> result;

        BOOL success = GetOpenFileNameA(&ofn);
        
        if (success)
        {
            // Parse the result buffer
            // Format: directory\0file1\0file2\0...\0\0
            const char* ptr = filename.data();
            std::string directory = ptr;
            ptr += directory.length() + 1;

            // Check if only one file was selected
            if (*ptr == '\0')
            {
                result.push_back(directory);
                LOG_DEBUG() << "Single file selected: " << directory;
            }
            else
            {
                // Multiple files selected
                while (*ptr != '\0')
                {
                    std::string file = ptr;
                    result.push_back(directory + "\\" + file);
                    LOG_DEBUG() << "File selected: " << (directory + "\\" + file);
                    ptr += file.length() + 1;
                }
            }
        }
        else
        {
            DWORD error = CommDlgExtendedError();
            if (error == 0)
            {
                LOG_DEBUG() << "User canceled file dialog";
            }
            else
            {
                LOG_ERROR() << "GetOpenFileNameA failed with error: " << error;
            }
        }

        return result;
    }

    std::optional<std::string> WindowsBrowserWindow::SaveFile(
        const std::string& title,
        const std::vector<FileFilter>& filters,
        const std::string& defaultPath,
        const std::string& defaultExtension)
    {
        LOG_DEBUG() << "SaveFile called: title=" << title << ", windowHandle=" << m_windowHandle;
        
        char filename[MAX_PATH] = { 0 };

        if (!defaultPath.empty())
        {
            strncpy_s(filename, MAX_PATH, defaultPath.c_str(), MAX_PATH - 1);
        }

        // Keep filter string alive for the duration of the dialog
        std::string filterStr = BuildFilterString(filters);

        OPENFILENAMEA ofn = { 0 };
        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.hwndOwner = static_cast<HWND>(m_windowHandle); // Can be NULL for no parent
        ofn.lpstrFile = filename;
        ofn.nMaxFile = MAX_PATH;
        ofn.lpstrFilter = filterStr.c_str();
        ofn.nFilterIndex = 1;
        ofn.lpstrTitle = title.c_str();
        ofn.Flags = OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

        if (!defaultExtension.empty())
        {
            ofn.lpstrDefExt = defaultExtension.c_str();
        }

        LOG_DEBUG() << "Calling GetSaveFileNameA...";
        BOOL result = GetSaveFileNameA(&ofn);
        
        if (result)
        {
            LOG_DEBUG() << "Save file selected: " << filename;
            return std::string(filename);
        }
        else
        {
            DWORD error = CommDlgExtendedError();
            if (error == 0)
            {
                LOG_DEBUG() << "User canceled save dialog";
            }
            else
            {
                LOG_ERROR() << "GetSaveFileNameA failed with CommDlgExtendedError: 0x" << std::hex << error << std::dec;
                LOG_ERROR() << "GetLastError: " << GetLastError();
                
                // Log more details about the failure
                if (error == 0xFFFF || error == 65535)
                {
                    LOG_ERROR() << "Error 0xFFFF indicates dialog initialization failure";
                    LOG_ERROR() << "Common causes: invalid window handle, wrong thread, or resource issues";
                }
            }
        }

        return std::nullopt;
    }

    std::optional<std::string> WindowsBrowserWindow::SelectFolder(
        const std::string& title,
        const std::string& defaultPath)
    {
        LOG_DEBUG() << "SelectFolder called: title=" << title;
        
        // Initialize COM for this thread if not already initialized
        HRESULT hrInit = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
        bool comInitialized = SUCCEEDED(hrInit);
        
        if (!comInitialized && hrInit != RPC_E_CHANGED_MODE)
        {
            LOG_ERROR() << "Failed to initialize COM: " << hrInit;
            return std::nullopt;
        }

        // Use the modern IFileDialog interface for folder selection
        IFileDialog* pfd = nullptr;
        HRESULT hr = CoCreateInstance(
            CLSID_FileOpenDialog,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_PPV_ARGS(&pfd));

        if (FAILED(hr) || !pfd)
        {
            LOG_ERROR() << "Failed to create file dialog: " << hr;
            if (comInitialized) CoUninitialize();
            return std::nullopt;
        }

        // Set options for folder picker
        DWORD dwOptions;
        hr = pfd->GetOptions(&dwOptions);
        if (SUCCEEDED(hr))
        {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS);
        }

        // Set title
        if (!title.empty())
        {
            std::wstring wTitle(title.begin(), title.end());
            pfd->SetTitle(wTitle.c_str());
        }

        // Set default folder if specified
        if (!defaultPath.empty())
        {
            IShellItem* psi = nullptr;
            std::wstring wPath(defaultPath.begin(), defaultPath.end());
            hr = SHCreateItemFromParsingName(wPath.c_str(), nullptr, IID_PPV_ARGS(&psi));
            if (SUCCEEDED(hr) && psi)
            {
                pfd->SetFolder(psi);
                psi->Release();
            }
        }

        // Show the dialog
        hr = pfd->Show(static_cast<HWND>(m_windowHandle));

        std::optional<std::string> result;
        if (SUCCEEDED(hr))
        {
            IShellItem* psi = nullptr;
            hr = pfd->GetResult(&psi);
            if (SUCCEEDED(hr) && psi)
            {
                PWSTR pszPath = nullptr;
                hr = psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr) && pszPath)
                {
                    // Convert wide string to narrow string
                    int size = WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, nullptr, 0, nullptr, nullptr);
                    if (size > 0)
                    {
                        std::string narrowPath(size - 1, '\0');
                        WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, &narrowPath[0], size, nullptr, nullptr);
                        result = narrowPath;
                        LOG_DEBUG() << "Folder selected: " << narrowPath;
                    }
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        else
        {
            if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED))
            {
                LOG_DEBUG() << "User canceled folder dialog";
            }
            else
            {
                LOG_ERROR() << "Folder dialog failed with error: " << hr;
            }
        }

        pfd->Release();
        
        if (comInitialized)
        {
            CoUninitialize();
        }
        
        return result;
    }

    bool WindowsBrowserWindow::DownloadFile(
        const std::string& filename,
        const void* data,
        size_t dataSize,
        const std::string& mimeType)
    {
        // On Windows, we use the native SaveFile dialog instead of browser download
        // This is a no-op since SaveFile already handles the native file saving
        LOG_DEBUG() << "DownloadFile not used on Windows platform (use SaveFile instead)";
        return false;
    }

} // namespace Core

#endif // _WIN32

#ifdef PLATFORM_WASM
#include <emscripten.h>
#include <functional>
#include <string>

namespace Core
{
    // Callback storage for async file operations
    static std::function<void(const std::string&)> g_fileLoadedCallback;
    static std::function<void(const std::string&)> g_fileSavedCallback;

    void WasmBrowserWindow::SetFileLoadedCallback(std::function<void(const std::string&)> callback) {
        g_fileLoadedCallback = std::move(callback);
    }

    void WasmBrowserWindow::SetFileSavedCallback(std::function<void(const std::string&)> callback) {
        g_fileSavedCallback = std::move(callback);
    }

    extern "C" {
        EMSCRIPTEN_KEEPALIVE
        void OnFileLoadedFromBrowser(const char* path) {
            if (g_fileLoadedCallback && path && strlen(path) > 0) {
                g_fileLoadedCallback(std::string(path));
                g_fileLoadedCallback = nullptr;
            }
        }

        EMSCRIPTEN_KEEPALIVE
        void OnFileSavedFromBrowser(const char* chosenName) {
            if (g_fileSavedCallback && chosenName && strlen(chosenName) > 0) {
                g_fileSavedCallback(std::string(chosenName));
                g_fileSavedCallback = nullptr;
            }
        }
    }

    std::optional<std::string> WasmBrowserWindow::OpenFile(
        const std::string& title,
        const std::vector<FileFilter>& filters,
        const std::string& defaultPath)
    {
        // Build accept attribute for file input from filters
        std::string accept;
        for (const auto& filter : filters) {
            if (!accept.empty()) accept += ",";
            // Convert pattern like "*.scene" to ".scene"
            std::string pattern = filter.pattern;
            size_t pos = pattern.find("*.");
            if (pos != std::string::npos) {
                pattern = pattern.substr(pos + 1); // Get ".scene"
            }
            accept += pattern;
        }
        if (accept.empty()) {
            accept = ".scene,*"; // Default to scene files
        }

        LOG_INFO() << "Opening browser file picker (non-blocking)";

        // Trigger file picker WITHOUT blocking - returns immediately
        EM_ASM({
            var accept = UTF8ToString($0);

            // Create a hidden file input element
            var input = document.createElement('input');
            input.type = 'file';
            input.accept = accept;
            input.style.display = 'none';
            document.body.appendChild(input);

            input.onchange = async function(e) {
                var file = e.target.files[0];
                if (!file) {
                    // User cancelled
                    document.body.removeChild(input);
                    return;
                }

                try {
                    // Read file as ArrayBuffer
                    const buffer = await file.arrayBuffer();
                    var data = new Uint8Array(buffer);

                    // Write file to Emscripten virtual filesystem
                    var path = '/tmp/' + file.name;

                    // Ensure /tmp directory exists
                    try {
                        FS.mkdir('/tmp');
                    } catch (e) {
                        // Directory might already exist
                    }

                    // Write the file
                    FS.writeFile(path, data);

                    // Call C++ callback with the path
                    var pathLen = lengthBytesUTF8(path) + 1;
                    var pathPtr = _malloc(pathLen);
                    stringToUTF8(path, pathPtr, pathLen);
                    Module._OnFileLoadedFromBrowser(pathPtr);
                    _free(pathPtr);

                    document.body.removeChild(input);
                    console.log('File loaded: ' + path + ' (' + data.length + ' bytes)');
                } catch (err) {
                    console.error('Error reading file:', err);
                    document.body.removeChild(input);
                }
            };

            // Trigger the file picker
            input.click();
        }, accept.c_str());

        // Return nullopt immediately - the callback will be called later
        return std::nullopt;
    }

    bool WasmBrowserWindow::DownloadFile(
        const std::string& filename,
        const void* data,
        size_t dataSize,
        const std::string& mimeType)
    {
        if (!data || dataSize == 0)
        {
            LOG_ERROR() << "Cannot download empty file";
            return false;
        }

        // JavaScript code to trigger browser save dialog
        // Uses modern File System Access API with fallback to direct download
        EM_ASM({
            var filename = UTF8ToString($0);
            var mimeType = UTF8ToString($1);
            var dataPtr = $2;
            var dataSize = $3;

            // Copy the data from WASM memory to a JavaScript array
            var dataArray = new Uint8Array(dataSize);
            dataArray.set(HEAPU8.subarray(dataPtr, dataPtr + dataSize));

            // Create a Blob from the data
            var blob = new Blob([dataArray], { type: mimeType });

            // Notify C++ of the chosen filename
            function notifySaved(chosenName) {
                var nameLen = lengthBytesUTF8(chosenName) + 1;
                var namePtr = _malloc(nameLen);
                stringToUTF8(chosenName, namePtr, nameLen);
                Module._OnFileSavedFromBrowser(namePtr);
                _free(namePtr);
            }

            if (window.showSaveFilePicker) {
                var extension = filename.split('.').pop() || 'scene';
                window.showSaveFilePicker({
                    suggestedName: filename,
                    types: [{
                        description: 'Scene Files',
                        accept: { 'application/octet-stream': ['.' + extension] }
                    }]
                }).then(function(fileHandle) {
                    // Notify C++ with the user-chosen filename before writing
                    notifySaved(fileHandle.name);
                    return fileHandle.createWritable();
                }).then(function(writable) {
                    return writable.write(blob).then(function() {
                        return writable.close();
                    });
                }).then(function() {
                    console.log('File saved successfully: ' + filename);
                }).catch(function(err) {
                    if (err.name !== 'AbortError') {
                        console.error('Error saving file:', err);
                        fallbackDownload();
                    }
                });
            } else {
                // Fallback: direct download, use the suggested filename
                fallbackDownload();
                notifySaved(filename);
            }

            function fallbackDownload() {
                var url = URL.createObjectURL(blob);
                var a = document.createElement('a');
                a.href = url;
                a.download = filename;
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                URL.revokeObjectURL(url);
                console.log('File download triggered (fallback): ' + filename);
            }

        }, filename.c_str(), mimeType.c_str(), data, dataSize);

        LOG_INFO() << "Browser download triggered: " << filename << " (" << dataSize << " bytes)";
        return true;
    }

} // namespace Core

#endif // PLATFORM_WASM
