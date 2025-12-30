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
            static Logger& instance() {
                static Logger inst;
                return inst;
            }

            void set_level(Level lvl) { level_ = lvl; }
			Level get_level() const { return level_; }
            void enable_console(bool enabled) { console_enabled_ = enabled; }
            void enable_gui_logging(bool enabled) { gui_logging_enabled_ = enabled; }
            void set_max_gui_logs(size_t max) { max_gui_logs_ = max; }
            
            bool open_file(const std::string& path) {
                std::lock_guard<std::mutex> lock(mu_);
                file_.close();
                file_.open(path, std::ios::out | std::ios::app);
                return file_.is_open();
            }

            bool should_log(Level lvl) const {
                return static_cast<int>(lvl) >= static_cast<int>(level_);
            }

            void log(Level lvl, const char* file, int line, const char* func, const std::string& msg) {
                if (!should_log(lvl)) return;
                auto now = std::chrono::system_clock::now();
                auto t = std::chrono::system_clock::to_time_t(now);
                
                std::tm tm_buf;
#ifdef _WIN32
                localtime_s(&tm_buf, &t);
                auto tm = &tm_buf;
#else
                auto tm = localtime_r(&t, &tm_buf);
#endif

                std::ostringstream oss;
                oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
                std::string timestamp = oss.str();

                std::ostringstream fullOss;
                fullOss << timestamp
                    << " [" << level_name(lvl) << "] "
                    << "(" << func << " " << file << ":" << line << ") "
                    << msg << "\n";
                auto lineStr = fullOss.str();

                std::lock_guard<std::mutex> lock(mu_);
                
                // Store for GUI display
                if (gui_logging_enabled_) {
                    gui_logs_.push_back({ timestamp, lvl, msg, file, line, func });
                    if (gui_logs_.size() > max_gui_logs_) {
                        gui_logs_.pop_front();
                    }
                }
                
                if (console_enabled_) std::cerr << lineStr;
                if (file_.is_open()) file_ << lineStr;
            }

            std::vector<LogEntry> get_gui_logs() const {
                std::lock_guard<std::mutex> lock(mu_);
                return std::vector<LogEntry>(gui_logs_.begin(), gui_logs_.end());
            }

            void clear_gui_logs() {
                std::lock_guard<std::mutex> lock(mu_);
                gui_logs_.clear();
            }

        private:
            Logger() = default;
            ~Logger() { file_.close(); }

            const char* level_name(Level lvl) const {
                switch (lvl) {
                case Level::Trace: return "TRACE";
                case Level::Debug: return "DEBUG";
                case Level::Info:  return "INFO";
                case Level::Warn:  return "WARN";
                case Level::Error: return "ERROR";
                case Level::Critical: return "CRIT";
                }
                return "?";
            }

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
            LogStream(Level lvl, const char* file, int line, const char* func)
                : level_(lvl), file_(file), line_(line), func_(func), enabled_(Logger::instance().should_log(lvl)) {}

            ~LogStream() {
                if (enabled_) {
                    Logger::instance().log(level_, file_, line_, func_, stream_.str());
                }
            }

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

        // Stream-based macros with zero overhead when log level is disabled
        // Usage: LOG_TRACE() << "Value: " << x << ", Name: " << name;
#define LOG_TRACE()    Core::Log::LogStream(Core::Log::Level::Trace,    __FILE__, __LINE__, __func__)
#define LOG_DEBUG()    Core::Log::LogStream(Core::Log::Level::Debug,    __FILE__, __LINE__, __func__)
#define LOG_INFO()     Core::Log::LogStream(Core::Log::Level::Info,     __FILE__, __LINE__, __func__)
#define LOG_WARN()     Core::Log::LogStream(Core::Log::Level::Warn,     __FILE__, __LINE__, __func__)
#define LOG_ERROR()    Core::Log::LogStream(Core::Log::Level::Error,    __FILE__, __LINE__, __func__)
#define LOG_CRITICAL() Core::Log::LogStream(Core::Log::Level::Critical, __FILE__, __LINE__, __func__)
    }
}
