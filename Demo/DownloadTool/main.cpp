#include "ThreadPool.h"
#include "Logger.h"
#include "DownloadTool.h"
#include <iostream>
#include <string>
#include <memory>
#include <future>

// 进度条观察者实现
class ProgressBarObserver : public DownloadObserver {
public:
    void onDownloadStarted(const std::string& url) override {
        std::cout << "开始下载: " << url << std::endl;
    }

    void onDownloadProgress(const std::string& url, double progress, double speed) override {
        const int barWidth = 50; // 进度条宽度
        int pos = static_cast<int>(barWidth * progress / 100.0);
        
        std::cout << "\r[";
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "=";
            else if (i == pos) std::cout << ">";
            else std::cout << " ";
        }
        
        std::cout << "] " << std::fixed << std::setprecision(1) 
                 << progress << "% " << std::setprecision(2)
                 << speed << " MB/s    " << std::flush;
    }

    void onDownloadCompleted(const std::string& url, size_t bytes, double duration) override {
        std::cout << std::endl << "下载完成: " 
                 << (bytes / (1024.0 * 1024.0)) << " MB，用时 " 
                 << duration << " 秒，平均速度 "
                 << ((bytes / (1024.0 * 1024.0)) / duration) << " MB/s" << std::endl;
    }

    void onDownloadError(const std::string& url, const std::string& error) override {
        std::cerr << std::endl << "下载错误: " << error << std::endl;
    }
};

int main() {
    // 初始化 libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    try {
        // 初始化 Logger（单例）和 ThreadPool
        Logger::getInstance("logs", Logger::Level::INFO);
        Logger::getInstance().log(Logger::Level::INFO, "下载工具启动");
        
        ThreadPool pool(4);
        Logger::getInstance().log(Logger::Level::INFO, "线程池初始化完成 (4线程)");

        // 创建下载工具实例和进度条观察者
        auto downloader = std::make_shared<DownloadTool>();
        auto progressObserver = std::make_shared<ProgressBarObserver>();
        downloader->addObserver(progressObserver);
        
        std::string command;
        std::vector<std::future<DownloadTool::DownloadResult>> activeTasks;
        
        std::cout << "====== 命令行下载工具 ======" << std::endl;
        std::cout << "输入URL开始下载 (输入'quit'退出)" << std::endl;
        
        while (true) {
            // 清理已完成的任务
            activeTasks.erase(
                std::remove_if(activeTasks.begin(), activeTasks.end(),
                    [](const auto& task) { 
                        return task.wait_for(std::chrono::seconds(0)) == std::future_status::ready; 
                    }),
                activeTasks.end()
            );
            
            std::cout << "\n> ";
            std::getline(std::cin, command);
            
            if (command == "quit" || command == "exit") {
                break;
            }
            
            if (!command.empty()) {
                std::string url = command;
                std::string outputFile = "";
                
                // 检查是否指定了输出文件名 (格式: URL > filename)
                size_t pos = command.find(" > ");
                if (pos != std::string::npos) {
                    url = command.substr(0, pos);
                    outputFile = command.substr(pos + 3);
                }
                
                // 提交下载任务
                try {
                    Logger::getInstance().log(Logger::Level::INFO, "提交下载任务: " + url);
                    std::cout << "正在准备下载..." << std::endl;
                    
                    activeTasks.push_back(pool.enqueue([downloader, url, outputFile]() {
                        return downloader->download(url, outputFile);
                    }));
                } catch (const std::exception& e) {
                    std::cerr << "提交任务失败: " << e.what() << std::endl;
                    Logger::getInstance().log(Logger::Level::ERROR, "提交任务失败: " + std::string(e.what()));
                }
            }
        }
        
        // 等待所有活跃的下载任务完成
        std::cout << "等待所有下载任务完成..." << std::endl;
        for (auto& task : activeTasks) {
            try {
                auto result = task.get();
                if (result.success) {
                    Logger::getInstance().log(
                        Logger::Level::INFO, 
                        "任务成功完成: " + std::to_string(result.bytesDownloaded) + " 字节"
                    );
                }
            } catch (const std::exception& e) {
                Logger::getInstance().log(Logger::Level::ERROR, "任务异常: " + std::string(e.what()));
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "程序错误: " << e.what() << std::endl;
        Logger::getInstance().log(Logger::Level::ERROR, "程序错误: " + std::string(e.what()));
    }

    // 清理 libcurl
    curl_global_cleanup();
    return 0;
}