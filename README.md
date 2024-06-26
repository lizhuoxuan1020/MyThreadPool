# threadpool

����C++11�ļ���̳߳أ�֧�ֱ�κ����������������ܹ���ȡִ�з���ֵ���̳߳ع���������̶߳��У�ÿ�η���һ�������һ���̣߳���֧���Զ��ͷŶ���Ŀ����̡߳�

### ��Ҫ����

- **��κ�������������֧��**�������ύ����������ĺ�������ķ����ʽ��
- **�Զ��ͷſ����߳�**�������ֵ������ִ��������̡߳�
- **���ʵ��**��������С������ǿ��

### ʹ��ʾ��

```cpp
thread_pool::ThreadPool pool;
auto result = pool.commit([](int a, int b) { return a + b; }, 1, 2);
std::cout << result.get() << std::endl;
```
## �ο�  
[������� ���� ��������̳߳�](https://www.cnblogs.com/chiangchou/p/thread-pool.html)   
[�̳߳���Ŀ1](https://github.com/progschj/ThreadPool/tree/master)    
[�̳߳���Ŀ2](https://github.com/alice-viola/ThreadPool/tree/master)  