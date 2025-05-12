#ifndef THREADPOOL_H
#define THREADPOOL_H

/*
 **************** 线程池 ****************
 设计目标：
    1. 支持异步任务提交和结果获取
    2. 管理固定数量的工作线程，复用线程以减少创建/销毁开销
    3. 线程安全，支持并发访问任务队列
    4. 优雅停止，析构时清理所有线程
*/

#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <vector>
#include <queue>

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop_(false) {
        // 初始化固定个数的工作线程，循环等待任务
        for (int i = 0; i < numThreads; ++i) {
            workers_.emplace_back([this] {  // 原地构造工作线程
                while (true) {
                    std::function<void()> task; // 任务无参数、无返回
                    {
                        std::unique_lock<std::mutex> lock(mtx_);
                        cv_.wait(lock, [this] {return stop_ || !tasks_.empty(); });
                        if (stop_ && tasks_.empty()) return; // 无任务时停止工作线程
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task(); // 释放锁执行任务（有可能任务较为耗时）
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            stop_ = true;   // 需要加锁以保护任务队列
        }
        cv_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // 异步提交任务，返回 std::future 以获取结果
    template<typename F, typename... Args>  // 可变参数模板（接受多个类型参数）
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        // 推断可调用对象的返回类型
        using ReturnType = std::invoke_result_t<F, Args...>;
        // 将一个零参数的可调用对象封装为异步任务
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...) // 完美转发 f 和参数 args...
        );
        std::future<ReturnType> result = task->get_future(); // 异步任务的结果容器
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if (stop_) throw std::runtime_error("Enqueue on stopped thread pool");
            tasks_.emplace([task] {(*task)(); });    // 原地构造异步执行的任务
        }
        cv_.notify_all();
        return result;
    }

private:
    std::vector<std::thread> workers_;   // 工作线程
    std::queue<std::function<void()>> tasks_;   // 任务队列（擦除可调用对象的类型）
    std::mutex mtx_;
    std::condition_variable cv_;
    bool stop_;
};


#endif // THREADPOOL_H