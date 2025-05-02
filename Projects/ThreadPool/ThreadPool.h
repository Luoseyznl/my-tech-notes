#include <iostream>
#include <functional>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(tasks_mutex);
                        condition.wait(lock, [this] {return stop || !tasks.empty();});
                        if (stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            stop = true;
        }

        condition.notify_all();
        for (std::thread& thread : workers) {
            thread.join();
        }
    }

    void submit(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(tasks_mutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }


private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mutex;
    std::condition_variable condition;
    bool stop = false;
};

