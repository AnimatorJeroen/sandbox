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

} // namespace Core

#endif // _WIN32
