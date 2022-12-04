#include "thread.h"

#include <thread>

size_t Thread::generatedId_ = 0;

Thread::Thread(threadFuntion threadFunc)
    : threadFunc_(threadFunc)
    , threadId_(generatedId_++) {
}

Thread::~Thread() {}

size_t Thread::getId() const {
  return threadId_;
}

// 启动线程
void Thread::start() {
  // 创建一个线程执行线程函数，出作用域后线程对象析构
  std::thread t(threadFunc_, threadId_);
  // 设置分离线程 (linux 中的 pthread_detach)
  t.detach();
}