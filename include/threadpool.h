#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <queue>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <future>
#include <iostream>
#include <unordered_map>
#include <condition_variable>

#include "noncopyable.h"
#include "thread.h"

enum class PoolMode {
  MODE_FIXED,
  MODE_CACHED,
};

class ThreadPool : noncopyable{
public:
  ThreadPool();
  ~ThreadPool();
  // 开启线程池
  void start(int initThreadSize = std::thread::hardware_concurrency());
  // 设置任务队列上限的阈值
  void setTaskQueueMaxSize(int maxSize_);

  // 生产任务，提交到任务队列，使用可变参模板
  // 返回值类型需要一个 future，但 future 实例化的类型需要推导
  template<typename Func, typename... Args>
  auto submitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))> {
    // 打包任务，放入任务队列
    using RtType = decltype(func(args...));
    auto task = std::make_shared<std::packaged_task<RtType()>>(
        std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
    std::future<RtType> result = task->get_future();
    // 获取锁
    std::unique_lock<std::mutex> lock(taskQueueMutex_);
    // 持续等待  持续等待指定时间    持续等待直到指定的时间
    //  wait  |    wait_for    |     wait_until
    // 用户提交任务阻塞时长不能超过 1s
    if (!queueNotFull_.wait_for(lock, std::chrono::seconds(1), \
        [&](){ return taskQueue_.size() < taskQueueMaxSize_; })) {
      // 因到达等待时长(设置为1s)而返回，但此时条件仍未满足
      std::cerr << "Task queue is full, submit task failed!\n";
      // 任务提交失败，随意提交一个任务
      auto task = std::make_shared<std::packaged_task<RtType()>>(
          []()->RtType { return RtType(); });
      return task->get_future();
    }
    //! 任务队列有空余可插入，由于任务队列中的任务返回值为void
    //! 所以在这里加一层中间层，利用一个 lambda 表达式匿名函数对象封装实际要执行的 task 任务
    taskQueue_.emplace([task](){ (*task)(); });
    ++taskSize_;

    // 放入新任务后，任务列表非空，通知消费线程消费任务
    queueNotEmpty_.notify_all();

    // 若处于 cached 模式，判断任务数量和空闲线程数量是否合理，以此决定是否创建新线程
    if (poolMode_ == PoolMode::MODE_CACHED
        && taskSize_ > idleThreadSize_
        && threadsMap_.size() < maxThreadSize_) {
      // 创建新线程对象
      std::cout << "Create new thread!\n";
      auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::threadFunc, this, std::placeholders::_1));
      size_t threadId = ptr->getId();
      threadsMap_.emplace(threadId, std::move(ptr));
      // 启动新线程
      threadsMap_[threadId]->start();
      ++idleThreadSize_;
    }
    // 返回任务的 Result 对象
    return result;
  }

  // 设置线程池工作模式
  void setMode(PoolMode mode = PoolMode::MODE_FIXED);
  // cached 模式下，设置线程池最大线程数
  void setMaxThreadSize(int maxSize);
  // 设置 cached 模式下多余线程的最长空闲时间
  void setMaxThreadFreeTime_(size_t time);
private:
  // 线程函数
  void threadFunc(size_t threadId);
  // 线程池运行状态
  bool isRunningState() const;

  std::unordered_map<size_t, std::unique_ptr<Thread>> threadsMap_;
  std::vector<std::unique_ptr<Thread> > threadVector_;  // 线程列表
  size_t initThreadSize_;               // 初始线程数
  size_t maxThreadSize_;                // 最大线程数
  size_t maxThreadFreeTime_;            // cached 模式下，多余线程的最大空闲时间
  std::atomic_int idleThreadSize_;      // 当前空闲线程数量

  // 每一个任务都是一个函数对象
  using Task = std::function<void()>;
  std::queue<Task> taskQueue_;  // 任务队列
  std::atomic_uint taskSize_;                     // 任务数量
  size_t taskQueueMaxSize_;                       // 任务队列上限

  std::mutex taskQueueMutex_;                     // 保证任务队列的线程安全
  std::condition_variable queueNotFull_;          // 任务队列未满
  std::condition_variable queueNotEmpty_;         // 任务队列未空
  std::condition_variable waitForWorkFinished_;   // 等待线程执行结束

  PoolMode poolMode_;                   // 当前线程工作模式
  std::atomic_bool isPoolRunning_;      // 当前线程是否启动
};


#endif