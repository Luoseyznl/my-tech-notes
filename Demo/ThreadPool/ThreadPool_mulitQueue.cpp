#include<iostream>
#include<thread>
#include<mutex>
#include<functional>
#include<condition_variable>
#include<vector>
#include<queue>
#include<future>
#include<random>
class ThreadPool{
    private:
        struct localQueue
        {
            std::mutex mtx;
            std::condition_variable cv;
            std::queue<std::function<void()>> tasks;
        };

        std::vector<std::thread> workers;//工作线程
        std::vector<localQueue> tasksQueue;//任务队列
        int localSize;//线程数量
        bool stop;//停止符号
    
    public:
        ThreadPool(int num):stop(false),localSize(num){
            for(int i = 0;i < num;i++){
                workers.emplace_back([this,i]{
                    while (true)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(this->tasksQueue[i].mtx);
                            tasksQueue[i].cv.wait(lock,[this,i]{return stop||!tasksQueue[i].tasks.empty();});
                            if(stop && tasksQueue[i].tasks.empty()){
                                return;
                            }
                            if(!tasksQueue[i].tasks.empty()){
                                task = tasksQueue[i].tasks.front();
                                tasksQueue[i].tasks.pop();
                            }
                        }
                        task();
                    }
                    
                })
            ;}
            
        }

        template<class F,class ...Args>
        auto enqueue(F&&f,Args&&...args)->std::future<typename std::result_of<F(Args...)>::type>{
            using return_type = typename std::result_of<F(Args...)>::type;
            auto task = std::make_shared<::packaged_task<return_type()>>(
                std::bind(std::forward<F>f, std::forward<Args>args...);
            );
            std::future<return_type> res = task->get_future();
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            int index = std::rand() % localSize;
            tasksQueue[index].tasks.push([this]{(*task)();});
            tasksQueue[index].cv.notify_one();
            return res;
            
        }

        ~ThreadPool(){

            stop = true;
            
            for(auto& q:tasksQueue){
                q.cv.notify_all();
            }

            for(int i = 0;i<localSize;i++){
                workers[i].join();
            }
        }
};