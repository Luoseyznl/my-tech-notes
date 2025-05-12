#include <iostream>
#include <future>
#include <thread>
#include <chrono>

void producer(std::promise<int> prom) {
    std::this_thread::sleep_for(std::chrono::seconds(2)); // 模拟耗时任务
    try {
        // 假设任务成功
        prom.set_value(42); // 设置结果
    } catch (...) {
        prom.set_exception(std::current_exception()); // 设置异常
    }
}

void consumer(std::future<int> fut) {
    try {
        std::cout << "Consumer waiting for result...\n";
        int result = fut.get(); // 阻塞等待结果
        std::cout << "Consumer received: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Consumer caught exception: " << e.what() << std::endl;
    }
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    // 启动生产者和消费者线程
    std::thread prod(producer, std::move(prom));
    std::thread cons(consumer, std::move(fut));
    
    prod.join();
    cons.join();
    
    return 0;
}