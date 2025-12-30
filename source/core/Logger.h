
// logger.h
#pragma once
#include <mutex>
#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Core {

    namespace Log {
        enum class Level { Trace, Debug, Info, Warn, Error, Critical };

        class Logger {
        public:
            static Logger& instance() {
                static Logger inst;
                return inst;
            }

            void set_level(Level lvl) { level_ = lvl; }
            void enable_console(bool enabled) { console_enabled_ = enabled; }
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
                auto tm = std::localtime(&t);

                std::ostringstream oss;
                oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S")
                    << " [" << level_name(lvl) << "] "
                    << "(" << func << " " << file << ":" << line << ") "
                    << msg << "\n";
                auto lineStr = oss.str();

                std::lock_guard<std::mutex> lock(mu_);
                if (console_enabled_) std::cerr << lineStr;
                if (file_.is_open()) file_ << lineStr;
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
            Level level_ = Level::Info;
            bool console_enabled_ = true;
        };

        // Convenience functions to avoid touching the singleton in call sites
        inline void SetLevel(Level lvl) { Logger::instance().set_level(lvl); }
        inline void EnableConsole(bool e) { Logger::instance().enable_console(e); }
        inline bool OpenFile(const std::string& path) { return Logger::instance().open_file(path); }

        // Macros to capture location without string concatenation at call site
#define LOG_AT(lvl, msg) \
    do { if (Core::Log::Logger::instance().should_log(lvl)) \
        Core::Log::Logger::instance().log(lvl, __FILE__, __LINE__, __func__, (msg)); } while(0)

#define LOG_TRACE(msg)    LOG_AT(Core::Log::Level::Trace,    (msg))
#define LOG_DEBUG(msg)    LOG_AT(Core::Log::Level::Debug,    (msg))
#define LOG_INFO(msg)     LOG_AT(Core::Log::Level::Info,     (msg))
#define LOG_WARN(msg)     LOG_AT(Core::Log::Level::Warn,     (msg))
#define LOG_ERROR(msg)    LOG_AT(Core::Log::Level::Error,    (msg))
#define LOG_CRITICAL(msg) LOG_AT(Core::Log::Level::Critical, (msg))
    }
}
