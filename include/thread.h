#ifndef THREAD_H
#define THREAD_H
#include <functional>

class Thread {
public:
  using threadFuntion = std::function<void(size_t)>;
  // 线程函数对象类型
  Thread(threadFuntion threadFunc);
  ~Thread();
  // 启动线程
  void start();
  // 获取线程 id
  size_t getId() const;
private:
  threadFuntion threadFunc_;
  static size_t generatedId_;
  size_t threadId_;
};


#endif