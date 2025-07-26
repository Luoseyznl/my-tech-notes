#include<iostream>
#include<functional>
#include<future>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<queue>
#include<vector>
#include<chrono>
class ThreadPool{
    private:
        struct localQueue
        {
            std::deque<std::function<void()>> tasks;//双向队列，任务可能被从尾部偷取
            std::condition_variable cv;
            std::mutex mtx;
        };
        
        std::vector<std::thread> workers;//工作线程
        std::vector<localQueue> taskQueue;//工作队列
        bool stop;
        int localSize;//线程数量
    
    public:
        ThreadPool(int num):stop(false),localSize(num){
            for(int i = 0;i < num;i++){
                
                    workers.emplace_back([this,i]{
                        while(true){
                        {   
                            std::function<void()> task;
                            std::unique_lock<std::mutex> lock(this->taskQueue[i].mtx);//给自己的任务队列加锁
                            bool notified = taskQueue[i].cv.wait_for(lock,std::chrono::milliseconds(0),[this,i]{
                                return stop || !taskQueue[i].tasks.empty();
                            });/*立即检查自己的队列是否为空，如果不为空，则继续持有自己队列的锁并执行任务；
                                否则释放自己队列的锁，且不阻塞
                            */
                           if(notified){//自己的任务队列不为空
                                if(stop && taskQueue[i].tasks.empty()){
                                    return;
                                }
                                if(!taskQueue[i].tasks.empty()){
                                    task = std::move(taskQueue[i].tasks.front());
                                    taskQueue[i].tasks.pop_front();
                                }
                           }else{//自己的任务队列为空且并未出现停止信号，则偷取任务
                                while (!taskQueue[i].cv.wait_for(lock,std::chrono::milliseconds(0),[this,i]{return stop || !taskQueue[i].tasks.empty();}))
                                //自己的任务队列一直为空才会一直循环想要偷取任务，否则循环结束
                                {   
                                    int num = std::rand() % localSize;//随机取其他线程序号
                                    std::unique_lock<std::mutex> lock(taskQueue[num].mtx,std::try_to_lock);//尝试加锁
                                    if(lock.owns_lock()){//加锁成功
                                        if(!taskQueue[num].tasks.empty()){//有任务可以窃取
                                            task = std::move(taskQueue[num].tasks.back());//从尾部窃取
                                            taskQueue[num].tasks.pop_back();
                                        }
                                    }else{//加锁失败
                                        std::this_thread::sleep_for(std::chrono::milliseconds(200));//等待200ms，防止一直轮询
                                        continue;
                                    }
                                }
                           }
                           if(task){task();}
                           
                        }
                    }
                    });
                    
                
            }
        }

};