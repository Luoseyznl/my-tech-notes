#include "ThreadPool.h"
#include <iostream>

int main() {
    ThreadPool pool(4);
    for (int i = 0; i < 10; ++i) {
        pool.submit([i] {
            std::cout << "Task " << i << " executed by thread " << std::this_thread::get_id() << std::endl;
        });
    }
    return 0;
}

