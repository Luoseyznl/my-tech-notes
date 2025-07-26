#ifndef DOWNLOAD_OBSERVER_H
#define DOWNLOAD_OBSERVER_H

#include <string>

// 观察者接口
class DownloadObserver {
public:
    virtual ~DownloadObserver() = default;
    virtual void onDownloadStarted(const std::string& url) = 0;
    virtual void onDownloadProgress(const std::string& url, double progress, double speed) = 0;
    virtual void onDownloadCompleted(const std::string& url, size_t bytes, double duration) = 0;
    virtual void onDownloadError(const std::string& url, const std::string& error) = 0;
};

#endif // DOWNLOAD_OBSERVER_H