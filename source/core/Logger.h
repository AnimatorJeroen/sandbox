#pragma once
#include <mutex>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <deque>
#include <vector>

namespace Core {

    namespace Log {
        enum class Level { Trace, Debug, Info, Warn, Error, Critical };

        struct LogEntry {
            std::string timestamp;
            Level level;
            std::string message;
            std::string file;
            int line;
            std::string func;
        };

        class Logger {
        public:
            static Logger& instance();

            void set_level(Level lvl);
            Level get_level() const;
            void enable_console(bool enabled);
            void enable_gui_logging(bool enabled);
            void set_max_gui_logs(size_t max);
            
            bool open_file(const std::string& path);
            bool should_log(Level lvl) const;
            void log(Level lvl, const char* file, int line, const char* func, const std::string& msg);
            std::vector<LogEntry> get_gui_logs() const;
            void clear_gui_logs();

        private:
            Logger() = default;
            ~Logger();

            const char* level_name(Level lvl) const;

            mutable std::mutex mu_;
            std::ofstream file_;
            std::deque<LogEntry> gui_logs_;
            Level level_ = Level::Info;
            bool console_enabled_ = true;
            bool gui_logging_enabled_ = false;
            size_t max_gui_logs_ = 1000;
        };

        // Stream-based helper class that defers message construction
        class LogStream {
        public:
            LogStream(Level lvl, const char* file, int line, const char* func);
            ~LogStream();

            // Allow any type that supports operator<< to be streamed
            template<typename T>
            LogStream& operator<<(const T& value) {
                if (enabled_) {
                    stream_ << value;
                }
                return *this;
            }

            // Prevent copying
            LogStream(const LogStream&) = delete;
            LogStream& operator=(const LogStream&) = delete;

        private:
            Level level_;
            const char* file_;
            int line_;
            const char* func_;
            bool enabled_;
            std::ostringstream stream_;
        };

        // Convenience functions to avoid touching the singleton in call sites
        inline void SetLevel(Level lvl) { Logger::instance().set_level(lvl); }
        inline Level GetLevel() { return Logger::instance().get_level(); }
        inline void EnableConsole(bool e) { Logger::instance().enable_console(e); }
        inline void EnableGuiLogging(bool e) { Logger::instance().enable_gui_logging(e); }
        inline void SetMaxGuiLogs(size_t max) { Logger::instance().set_max_gui_logs(max); }
        inline bool OpenFile(const std::string& path) { return Logger::instance().open_file(path); }
        inline std::vector<LogEntry> GetGuiLogs() { return Logger::instance().get_gui_logs(); }
        inline void ClearGuiLogs() { Logger::instance().clear_gui_logs(); }

        // When hot reload is enabled, use simple logging without file/line/function info
        // to avoid issues with Edit and Continue
#ifdef DEBUG
#define LOG_TRACE()    Core::Log::LogStream(Core::Log::Level::Trace,    "", 0, "")
#define LOG_DEBUG()    Core::Log::LogStream(Core::Log::Level::Debug,    "", 0, "")
#define LOG_INFO()     Core::Log::LogStream(Core::Log::Level::Info,     "", 0, "")
#define LOG_WARN()     Core::Log::LogStream(Core::Log::Level::Warn,     "", 0, "")
#define LOG_ERROR()    Core::Log::LogStream(Core::Log::Level::Error,    "", 0, "")
#define LOG_CRITICAL() Core::Log::LogStream(Core::Log::Level::Critical, "", 0, "")
#else
#define LOG_TRACE()    Core::Log::LogStream(Core::Log::Level::Trace,    __FILE__, __LINE__, __func__)
#define LOG_DEBUG()    Core::Log::LogStream(Core::Log::Level::Debug,    __FILE__, __LINE__, __func__)
#define LOG_INFO()     Core::Log::LogStream(Core::Log::Level::Info,     __FILE__, __LINE__, __func__)
#define LOG_WARN()     Core::Log::LogStream(Core::Log::Level::Warn,     __FILE__, __LINE__, __func__)
#define LOG_ERROR()    Core::Log::LogStream(Core::Log::Level::Error,    __FILE__, __LINE__, __func__)
#define LOG_CRITICAL() Core::Log::LogStream(Core::Log::Level::Critical, __FILE__, __LINE__, __func__)
#endif
    }
}
