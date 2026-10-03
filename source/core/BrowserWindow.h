#pragma once
#include <string>
#include <vector>
#include <optional>

namespace Core
{
    // Filter for file dialog (e.g., "Text Files", "*.txt")
    struct FileFilter
    {
        std::string description;
        std::string pattern;

        FileFilter(const std::string& desc, const std::string& pat)
            : description(desc), pattern(pat) {}
    };

    // Platform-agnostic interface for OS file browser windows
    class IBrowserWindow
    {
    public:
        virtual ~IBrowserWindow() = default;

        // Open a file dialog to select a single file
        // Returns the selected file path, or std::nullopt if canceled
        virtual std::optional<std::string> OpenFile(
            const std::string& title = "Open File",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "") = 0;

        // Open a file dialog to select multiple files
        // Returns a vector of selected file paths, empty if canceled
        virtual std::vector<std::string> OpenFiles(
            const std::string& title = "Open Files",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "") = 0;

        // Open a file dialog to save a file
        // Returns the selected file path, or std::nullopt if canceled
        virtual std::optional<std::string> SaveFile(
            const std::string& title = "Save File",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "",
            const std::string& defaultExtension = "") = 0;

        // Open a folder selection dialog
        // Returns the selected folder path, or std::nullopt if canceled
        virtual std::optional<std::string> SelectFolder(
            const std::string& title = "Select Folder",
            const std::string& defaultPath = "") = 0;

        // Download a file to the browser (WASM-specific, no-op on other platforms)
        // Returns true if download was triggered successfully
        virtual bool DownloadFile(
            const std::string& filename,
            const void* data,
            size_t dataSize,
            const std::string& mimeType = "application/octet-stream") = 0;
    };

#ifdef _WIN32
    // Windows implementation using Common Dialog API
    class WindowsBrowserWindow : public IBrowserWindow
    {
    public:
        WindowsBrowserWindow(void* windowHandle = nullptr);
        ~WindowsBrowserWindow() override = default;

        std::optional<std::string> OpenFile(
            const std::string& title = "Open File",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "") override;

        std::vector<std::string> OpenFiles(
            const std::string& title = "Open Files",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "") override;

        std::optional<std::string> SaveFile(
            const std::string& title = "Save File",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "",
            const std::string& defaultExtension = "") override;

        std::optional<std::string> SelectFolder(
            const std::string& title = "Select Folder",
            const std::string& defaultPath = "") override;

        bool DownloadFile(
            const std::string& filename,
            const void* data,
            size_t dataSize,
            const std::string& mimeType = "application/octet-stream") override;

    private:
        void* m_windowHandle; // HWND stored as void* to avoid including Windows.h in header

        // Helper to convert filters to Windows format
        std::string BuildFilterString(const std::vector<FileFilter>& filters) const;
    };

    // Convenience typedef for the platform-specific implementation
    using BrowserWindow = WindowsBrowserWindow;
#elif defined(PLATFORM_WASM)
    // WASM implementation using browser APIs for file operations
    class WasmBrowserWindow : public IBrowserWindow
    {
    public:
        WasmBrowserWindow(void* = nullptr) {}

        // Open a file using browser file picker
        std::optional<std::string> OpenFile(
            const std::string& title = "Open File",
            const std::vector<FileFilter>& filters = {},
            const std::string& defaultPath = "") override;

        std::vector<std::string>   OpenFiles(const std::string& = {}, const std::vector<FileFilter>& = {}, const std::string& = {}) override { return {}; }
        std::optional<std::string> SaveFile(const std::string& = {}, const std::vector<FileFilter>& = {}, const std::string& = {}, const std::string& = {}) override { return std::nullopt; }
        std::optional<std::string> SelectFolder(const std::string& = {}, const std::string& = {}) override { return std::nullopt; }

        // Trigger a browser download for the given file data
        bool DownloadFile(
            const std::string& filename,
            const void* data,
            size_t dataSize,
            const std::string& mimeType = "application/octet-stream") override;

        // Set callback for when file is loaded (OpenFile)
        static void SetFileLoadedCallback(std::function<void(const std::string&)> callback);

        // Set callback for when file is saved - receives the actual filename chosen by the user
        static void SetFileSavedCallback(std::function<void(const std::string&)> callback);
    };
    using BrowserWindow = WasmBrowserWindow;
#else
    // For non-Windows platforms, you would implement other platform-specific versions here
    // e.g., LinuxBrowserWindow, MacBrowserWindow, etc.
    #error "BrowserWindow not implemented for this platform"
#endif

} // namespace Core
