#include "pch.h"
#include "Logger.h"

namespace Core {
    namespace Log {

        Logger& Logger::instance() {
            static Logger inst;
            return inst;
        }

        void Logger::set_level(Level lvl) {
            level_ = lvl;
        }

        Level Logger::get_level() const {
            return level_;
        }

        void Logger::enable_console(bool enabled) {
            console_enabled_ = enabled;
        }

        void Logger::enable_gui_logging(bool enabled) {
            gui_logging_enabled_ = enabled;
        }

        void Logger::set_max_gui_logs(size_t max) {
            max_gui_logs_ = max;
        }

        bool Logger::open_file(const std::string& path) {
            std::lock_guard<std::mutex> lock(mu_);
            file_.close();
            file_.open(path, std::ios::out | std::ios::app);
            return file_.is_open();
        }

        bool Logger::should_log(Level lvl) const {
            return static_cast<int>(lvl) >= static_cast<int>(level_);
        }

        void Logger::log(Level lvl, const char* file, int line, const char* func, const std::string& msg) {
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

        std::vector<LogEntry> Logger::get_gui_logs() const {
            std::lock_guard<std::mutex> lock(mu_);
            return std::vector<LogEntry>(gui_logs_.begin(), gui_logs_.end());
        }

        void Logger::clear_gui_logs() {
            std::lock_guard<std::mutex> lock(mu_);
            gui_logs_.clear();
        }

        Logger::~Logger() {
            file_.close();
        }

        const char* Logger::level_name(Level lvl) const {
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

        // LogStream implementation
        LogStream::LogStream(Level lvl, const char* file, int line, const char* func)
            : level_(lvl), file_(file), line_(line), func_(func), enabled_(Logger::instance().should_log(lvl)) {}

        LogStream::~LogStream() {
            if (enabled_) {
                Logger::instance().log(level_, file_, line_, func_, stream_.str());
            }
        }

    } // namespace Log
} // namespace Core
