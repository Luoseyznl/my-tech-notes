#ifndef DOWNLOADTOOL_H
#define DOWNLOADTOOL_H

/*
 **************************** 下载工具 ****************************
 设计目标：
    1. 封装 libcurl（进行 HTTP 下载），使用 ThreadPool 提交异步任务
    2. 提供下载字节数、时间、速度和进度信息
    3. 捕获 libcurl 错误和网络异常，通过 Logger 记录关键事件（开始、进度、完成、错误）
*/

#include "Logger.h"
#include "DownloadObserver.h"
#include <curl/curl.h>
#include <string>
#include <chrono>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <memory>

class DownloadTool
{
public:
    struct DownloadResult
    {
        size_t bytesDownloaded; // 下载字节数
        double durationSeconds; // 下载耗时（秒）
        double speedMbps;       // 下载速度（MB/s）
        bool success;           // 是否成功
        std::string error;      // 错误信息（如果失败）
    };

    DownloadTool() : curl_(nullptr)
    {
        curl_ = curl_easy_init(); // 创建并初始化一个easy句柄（CURL句柄）
        if (!curl_)
        {
            Logger::getInstance().log(Logger::Level::ERROR, "Failed to initialize curl");
            throw std::runtime_error("Failed to initialize curl");
        }
    }

    ~DownloadTool()
    {
        if (curl_)
        {
            curl_easy_cleanup(curl_);
        }
    }

    // 添加观察者
    void addObserver(std::shared_ptr<DownloadObserver> observer) {
        observers_.push_back(observer);
    }

    // 移除观察者
    void removeObserver(std::shared_ptr<DownloadObserver> observer) {
        observers_.erase(
            std::remove_if(observers_.begin(), observers_.end(), 
                [&](const std::shared_ptr<DownloadObserver>& o) { 
                    return o == observer; 
                }),
            observers_.end()
        );
    }

    // 下载函数：执行 HTTP 下载，保存到文件，返回结果
    DownloadResult download(const std::string &url, const std::string &outputFile = "")
    {
        // 确定输出文件名
        std::string finalOutput = outputFile.empty() ? extractFileName(url) : outputFile;
        
        // 通知开始下载
        notifyDownloadStarted(url);
        Logger::getInstance().log(Logger::Level::INFO, "Starting download: " + url + " to " + finalOutput);

        // 打开输出文件
        outFile_.open(finalOutput, std::ios::binary);
        if (!outFile_.is_open()) {
            std::string error = "Failed to open output file: " + finalOutput;
            notifyDownloadError(url, error);
            Logger::getInstance().log(Logger::Level::ERROR, error);
            return {0, 0, 0, false, error};
        }

        auto start = std::chrono::steady_clock::now();

        // 设置 curl 参数
        curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl_, CURLOPT_WRITEDATA, this);
        curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl_, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl_, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl_, CURLOPT_XFERINFODATA, this);
        curl_easy_setopt(curl_, CURLOPT_FAILONERROR, 1L);
        
        currentUrl_ = url; // 保存当前URL用于回调
        bytesDownloaded_ = 0;
        lastProgress_ = -1.0;
        
        // 执行下载
        CURLcode res = curl_easy_perform(curl_);
        outFile_.close();
        
        auto end = std::chrono::steady_clock::now();
        double duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
        double speedMbps = (bytesDownloaded_ / (1024.0 * 1024.0)) / duration;

        if (res != CURLE_OK) {
            std::string error = "Download failed: " + std::string(curl_easy_strerror(res));
            Logger::getInstance().log(Logger::Level::ERROR, error);
            notifyDownloadError(url, error);
            return {bytesDownloaded_, duration, speedMbps, false, error};
        }

        std::stringstream ss;
        ss << "Download completed: " << std::fixed << std::setprecision(2)
           << bytesDownloaded_ / (1024.0 * 1024.0) << " MB, Time: " << duration
           << " s, Speed: " << speedMbps << " MB/s, Saved to: " << finalOutput;
        
        Logger::getInstance().log(Logger::Level::INFO, ss.str());
        notifyDownloadCompleted(url, bytesDownloaded_, duration);
        
        return {bytesDownloaded_, duration, speedMbps, true, ""};
    }

private:
    CURL *curl_;             // curl 句柄
    std::ofstream outFile_;  // 输出文件流
    size_t bytesDownloaded_; // 下载字节数
    double lastProgress_;    // 上次进度（避免重复日志）
    std::string currentUrl_; // 当前下载URL
    std::vector<std::shared_ptr<DownloadObserver>> observers_; // 观察者列表

    // 通知观察者下载开始
    void notifyDownloadStarted(const std::string& url) {
        for (auto& observer : observers_) {
            observer->onDownloadStarted(url);
        }
    }

    // 通知观察者下载进度
    void notifyDownloadProgress(const std::string& url, double progress, double speed) {
        for (auto& observer : observers_) {
            observer->onDownloadProgress(url, progress, speed);
        }
    }

    // 通知观察者下载完成
    void notifyDownloadCompleted(const std::string& url, size_t bytes, double duration) {
        for (auto& observer : observers_) {
            observer->onDownloadCompleted(url, bytes, duration);
        }
    }

    // 通知观察者下载错误
    void notifyDownloadError(const std::string& url, const std::string& error) {
        for (auto& observer : observers_) {
            observer->onDownloadError(url, error);
        }
    }

    // 从 URL 提取文件名
    std::string extractFileName(const std::string &url)
    {
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash == std::string::npos || lastSlash == url.size() - 1) {
            return "download_" + std::to_string(
                std::chrono::system_clock::now().time_since_epoch().count()
            );
        }
        return url.substr(lastSlash + 1);
    }

    // 写入回调：将数据写入文件
    static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp)
    {
        DownloadTool *tool = static_cast<DownloadTool *>(userp);
        size_t totalSize = size * nmemb;
        tool->outFile_.write(static_cast<char *>(contents), totalSize);
        if (!tool->outFile_) {
            return 0;  // 表示写入错误
        }
        tool->bytesDownloaded_ += totalSize;
        return totalSize;
    }

    // 进度回调：记录并输出进度
    static int progressCallback(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t, curl_off_t)
    {
        DownloadTool *tool = static_cast<DownloadTool *>(clientp);
        if (dltotal > 0) {
            double progress = static_cast<double>(dlnow) / static_cast<double>(dltotal) * 100.0;
            
            // 计算速度 (简单实现)
            static auto lastTime = std::chrono::steady_clock::now();
            static curl_off_t lastBytes = 0;
            
            auto now = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastTime).count() / 1000.0;
            
            double speed = 0;
            if (duration > 0.1) {  // 至少经过0.1秒才更新速度
                speed = (dlnow - lastBytes) / (1024.0 * 1024.0) / duration; // MB/s
                lastTime = now;
                lastBytes = dlnow;
            }
            
            // 按10%的步长记录进度
            if (progress - tool->lastProgress_ >= 10.0 || progress >= 100.0) {
                std::stringstream ss;
                ss << "Download progress: " << std::fixed << std::setprecision(1) << progress 
                   << "%, " << std::setprecision(2) << speed << " MB/s";
                
                Logger::getInstance().log(Logger::Level::INFO, ss.str());
                tool->notifyDownloadProgress(tool->currentUrl_, progress, speed);
                tool->lastProgress_ = progress;
            }
        }
        return 0; // 返回非0值会中止传输
    }
};

#endif // DOWNLOADTOOL_H