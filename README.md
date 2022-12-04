# Thread_Pooling_CPP17

#### Build:

```
$ ./autobuild.sh
```

#### Use:

```c++
#include "threadpool.h"

// task_1
int add1(int a, int b) {
  return a + b;
}

// task_2
int add2(int a, int b, int c) {
  return a + b + c;
}

int main() {
  ThreadPool pool;
  // Default mode is MODE_FIXED:set thread nums by machine core nums
  // MODE_CACHED:auto create new thread when you submit too many tasks
  pool.setMode(PoolMode::MODE_CACHED);
  // if work on MODE_CACHED, the thread created will die after the time you set(seconds)
  pool.setMaxThreadFreeTime_(60);
  // set init thread nums(default is machine core nums)
  pool.start(8);
  std::future<int> res1 = pool.submitTask(add1, 1, 2);
  std::future<int> res2 = pool.submitTask(add2, 1, 2, 3);
  // you can also submit task by lambda
  std::future<int> res3 = pool.submitTask([](int a, int b){
    int sum = 0;
    for (int i = a; i <= b; ++i) {
      sum += i;
    }
    return sum;
  }, 1, 100);
}
  
```

