#include<iostream>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
#include<vector>
#include<queue>
#include<future>

class ThreadPool{
    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable cv;
        bool stop;
    
    public:
        ThreadPool(int num):stop(false){
            for(int i = 0;i < num;i++){
                workers.emplace_back([this]{
                    while(true){
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->mtx);
                            this->cv.wait(lock,[this]{return (stop || !this->tasks.empty());});
                            if(stop && this->tasks.empty()){
                                return;
                            }
                            if(!this->tasks.empty()){
                                task = std::move(tasks.front());
                                tasks.pop();
                            }
                        }
                        task();
                    }
                });
            }
        }
        template<class F,class ...Args>
        auto enqueue(F&&f,Args&&...args)->std::future<typename std::result_of<F(Args...)>::type>{
            using return_type = typename std::result_of<F(Args...)>::type;
            return_type res;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                bind(std::forward<F>f, std::forward<Arg...>args)
            );
            return_type res = task->get_future();
            {
                std::unique_lock<mutex> lock(this->mtx);
                tasks.push([task]{(*task)()};);
            }
            cv.notify_one();
            
            return res;
        }

        ~ThreadPool(){
            {
                std::unique_lock<std::mutex> lock(mtx);
                stop = true;
            }
            cv.notify_all();
            for(std::thread& worker:workers){
                worker.join();
            }
            
        }
};