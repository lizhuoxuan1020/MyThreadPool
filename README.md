# threadpool

基于C++11的简洁线程池，支持变参函数和匿名函数，能够获取执行返回值。线程池管理任务和线程队列，每次分配一个任务给一个线程，并支持自动释放多余的空闲线程。

### 主要特性

- **变参函数和匿名函数支持**：可以提交带任意参数的函数或拉姆达表达式。
- **自动释放空闲线程**：避免峰值过后出现大量空闲线程。
- **简洁实现**：代码量小，功能强大。

### 使用示例

```cpp
thread_pool::ThreadPool pool;
auto result = pool.commit([](int a, int b) { return a + b; }, 1, 2);
std::cout << result.get() << std::endl;
```
## 参考  
[并发编程 —— 深入理解线程池](https://www.cnblogs.com/chiangchou/p/thread-pool.html)   
[线程池项目1](https://github.com/progschj/ThreadPool/tree/master)    
[线程池项目2](https://github.com/alice-viola/ThreadPool/tree/master)  