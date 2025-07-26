#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <sstream>
#include <chrono>
#include <filesystem>
#include <queue>
#include <future>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

class Logger
{
public:
    enum class Level
    {
        DEFAULT,
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // 获取单例实例
    static Logger &getInstance(const std::string &logDir = "logs", Level minLevel = Level::INFO,
                                size_t maxFileSize = 250 * 1024, size_t maxFiles = 100, size_t bufferSize = 100)
    {
        static std::once_flag onceFlag;
        static std::unique_ptr<Logger> instance;

        std::call_once(onceFlag, [&]() {
            instance.reset(new Logger(logDir, minLevel, maxFileSize, maxFiles, bufferSize));
        });

        return *instance;
    }

    Logger(const Logger &) = delete; // 禁止拷贝构造函数
    Logger &operator=(const Logger &) = delete; // 禁止赋值操作符
    Logger(Logger &&) = delete; // 禁止移动构造函数
    Logger &operator=(Logger &&) = delete; // 禁止移动赋值操作符

    // 日志函数：记录日志消息
    void log(Level level, const std::string &message)
    {
        if (level < minLevel_)
            return;

        std::string logEntry = formatLogEntry(level, message);
    
        // 生产者-消费者模式：生产者生产日志
        {
            std::lock_guard<std::mutex> lock(mtx_);
            buffer_.push(logEntry);  // 使用队列代替vector
            if (buffer_.size() >= bufferSize_) {
                cv_.notify_one();
            }
        }
    }

    // 析构函数：停止写入线程，写入剩余日志
    ~Logger()
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;
        }
        cv_.notify_all();
        if (writerThread_.joinable())
        {
            writerThread_.join();
        }
        flushBuffer(); // 写入剩余日志
    }

private:
    // 私有构造函数（单例模式）：初始化目录、级别、参数
    Logger(const std::string &logDir = "logs", Level minLevel = Level::INFO,
           size_t maxFileSize = 250 * 1024, size_t maxFiles = 100, size_t bufferSize = 100)
        : logDir_(logDir), minLevel_(minLevel), maxFileSize_(maxFileSize), maxFiles_(maxFiles),
          bufferSize_(bufferSize), stop_(false), currentFileSize_(0)
    {
        std::filesystem::create_directory(logDir_);
        writerThread_ = std::thread(&Logger::writerLoop, this);
    }

    std::string logDir_;              // 日志目录
    Level minLevel_;                  // 最低日志等级
    size_t maxFileSize_;              // 单文件最大大小（字节）
    size_t maxFiles_;                 // 最大文件数
    size_t bufferSize_;               // 缓冲区大小
    std::queue<std::string> buffer_;  // 改用队列实现生产者-消费者模式
    std::ofstream logFile_;           // 当前日志文件
    size_t currentFileSize_;          // 当前文件大小
    std::string currentFileName_;     // 当前文件名

    std::thread writerThread_;        // 写入线程
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_;

    // 辅助函数：格式化日志条目
    std::string formatLogEntry(Level level, const std::string &message)
    {
        static const std::string levelStr[] = {"DEFAULT", "DEBUG", "INFO", "WARNING", "ERROR"};
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
           << " [" << levelStr[static_cast<int>(level)] << "] " << message;
        return ss.str();
    }
    // 辅助函数：生成新日志文件名
    std::string generateFileName()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << logDir_ << "/log_" << std::put_time(std::localtime(&time), "%Y%m%d_%H%M%S")
           << "_" << std::setw(3) << std::setfill('0') << getFileCount() << ".log";
        return ss.str();
    }
    // 辅助函数：获取当前日志文件数
    size_t getFileCount()
    {
        std::vector<std::string> files;
        for (const auto &entry : std::filesystem::directory_iterator(logDir_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".log")
            {
                files.push_back(entry.path().string());
            }
        }
        return files.size();
    }
    // 辅助函数：删除最旧日志文件
    void deleteOldestFile()
    {
        std::vector<std::string> files;
        for (const auto &entry : std::filesystem::directory_iterator(logDir_))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".log")
            {
                files.push_back(entry.path().string());
            }
        }
        if (!files.empty())
        {
            std::sort(files.begin(), files.end());
            std::filesystem::remove(files.front()); // 删除最旧文件
        }
    }
    // 辅助函数：写入缓冲区到文件（消费者模式）
    void flushBuffer()
    {
        if (buffer_.empty())
            return;
        if (!logFile_.is_open())
        {
            openNewFile();
        }
        while (!buffer_.empty())
        {
            std::string entry = buffer_.front();
            buffer_.pop();
            size_t entrySize = entry.size() + 1; // 包括换行符
            if (currentFileSize_ + entrySize > maxFileSize_)
            {
                openNewFile();
            }
            logFile_ << entry << std::endl;
            currentFileSize_ += entrySize;
        }
        logFile_.flush();
    }
    // 辅助函数：打开新日志文件
    void openNewFile()
    {
        if (logFile_.is_open())
        {
            logFile_.close();
        }
        if (getFileCount() >= maxFiles_)
        {
            deleteOldestFile();
        }
        currentFileName_ = generateFileName();
        logFile_.open(currentFileName_, std::ios::app);
        if (!logFile_.is_open())
        {
            throw std::runtime_error("Failed to open log file: " + currentFileName_);
        }
        currentFileSize_ = 0;
    }
    // 辅助函数：写入线程循环
    void writerLoop()
    {
        while (!stop_)
        {
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock, [this] { return !buffer_.empty() || stop_; });
            flushBuffer();
        }
        flushBuffer(); // 最后一次刷新
    }
};

#endif // LOGGER_H