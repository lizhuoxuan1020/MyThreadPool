#pragma once
#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>
#include <queue>
#include <atomic>
#include <future>
#include <condition_variable>
#include <thread>
#include <functional>
#include <stdexcept>

namespace thread_pool{
	//�̳߳��������,Ӧ������Сһ��
#define  THREADPOOL_MAX_NUM 16
//�̳߳��Ƿ�����Զ�����(�����Ҫ,�Ҳ����� THREADPOOL_MAX_NUM)
//#define  THREADPOOL_AUTO_GROW

//�̳߳�,�����ύ��κ�������ķ����ʽ����������ִ��,���Ի�ȡִ�з���ֵ
//��ֱ��֧�����Ա����, ֧���ྲ̬��Ա������ȫ�ֺ���,Opteron()������
	class threadpool
	{
		unsigned short _initSize;       //��ʼ���߳�����
		using Task = std::function<void()>; //��������
		std::vector<std::thread> _pool;          //�̳߳�
		std::queue<Task> _tasks;            //�������
		std::mutex _lock;                   //�������ͬ����
#ifdef THREADPOOL_AUTO_GROW
		mutex _lockGrow;               //�̳߳�����ͬ����
#endif // !THREADPOOL_AUTO_GROW
		std::condition_variable _task_cv;   //��������
		std::atomic<bool> _run{ true };     //�̳߳��Ƿ�ִ��
		std::atomic<int>  _idlThrNum{ 0 };  //�����߳�����

	public:
		inline threadpool(unsigned short size = 4) { _initSize = size; addThread(size); }
		inline ~threadpool()
		{
			_run = false;
			_task_cv.notify_all(); // ���������߳�ִ��
			for (std::thread& thread : _pool) {
				//thread.detach(); // ���̡߳���������
				if (thread.joinable())
					thread.join(); // �ȴ���������� ǰ�᣺�߳�һ����ִ����
			}
		}

	public:
		// �ύһ������
		// ����.get()��ȡ����ֵ��ȴ�����ִ����,��ȡ����ֵ
		// �����ַ�������ʵ�ֵ������Ա��
		// һ����ʹ��   bind�� .commit(std::bind(&Dog::sayHello, &dog));
		// һ������   mem_fn�� .commit(std::mem_fn(&Dog::sayHello), this)
		template<class F, class... Args>
		auto commit(F&& f, Args&&... args) -> std::future<decltype(f(args...))>
		{
			if (!_run)    // stoped ??
				throw runtime_error("commit on ThreadPool is stopped.");

			using RetType = decltype(f(args...)); // typename std::result_of<F(Args...)>::type, ���� f �ķ���ֵ����
			auto task = make_shared<packaged_task<RetType()>>(
				bind(forward<F>(f), forward<Args>(args)...)
				); // �Ѻ�����ڼ�����,���(��)
			future<RetType> future = task->get_future();
			{    // ������񵽶���
				lock_guard<mutex> lock{ _lock };//�Ե�ǰ���������  lock_guard �� mutex �� stack ��װ�࣬�����ʱ�� lock()��������ʱ�� unlock()
				_tasks.emplace([task]() { // push(Task{...}) �ŵ����к���
					(*task)();
					});
			}
#ifdef THREADPOOL_AUTO_GROW
			if (_idlThrNum < 1 && _pool.size() < THREADPOOL_MAX_NUM)
				addThread(1);
#endif // !THREADPOOL_AUTO_GROW
			_task_cv.notify_one(); // ����һ���߳�ִ��

			return future;
		}
		// �ύһ���޲�����, ���޷���ֵ
		template <class F>
		void commit2(F&& task)
		{
			if (!_run) return;
			{
				lock_guard<mutex> lock{ _lock };
				_tasks.emplace(std::forward<F>(task));
			}
#ifdef THREADPOOL_AUTO_GROW
			if (_idlThrNum < 1 && _pool.size() < THREADPOOL_MAX_NUM)
				addThread(1);
#endif // !THREADPOOL_AUTO_GROW
			_task_cv.notify_one();
		}
		//�����߳�����
		int idlCount() { return _idlThrNum; }
		//�߳�����
		int thrCount() { return _pool.size(); }

#ifndef THREADPOOL_AUTO_GROW
	private:
#endif // !THREADPOOL_AUTO_GROW
		//���ָ���������߳�
		void addThread(unsigned short size)
		{
#ifdef THREADPOOL_AUTO_GROW
			if (!_run)    // stoped ??
				throw runtime_error("Grow on ThreadPool is stopped.");
			unique_lock<mutex> lockGrow{ _lockGrow }; //�Զ�������
#endif // !THREADPOOL_AUTO_GROW
			for (; _pool.size() < THREADPOOL_MAX_NUM && size > 0; --size)
			{   //�����߳�����,�������� Ԥ�������� THREADPOOL_MAX_NUM
				_pool.emplace_back([this] { //�����̺߳���
					while (true) //��ֹ _run==false ʱ��������,��ʱ������п��ܲ�Ϊ��
					{
						Task task; // ��ȡһ����ִ�е� task
						{
							// unique_lock ��� lock_guard �ĺô��ǣ�������ʱ unlock() �� lock()
							std::unique_lock<std::mutex> lock{ _lock };
							_task_cv.wait(lock, [this] { // wait ֱ���� task, ����Ҫֹͣ
								return !_run || !_tasks.empty();
								});
							if (!_run && _tasks.empty())
								return;
							_idlThrNum--;
							task = std::move(_tasks.front()); // ���Ƚ��ȳ��Ӷ���ȡһ�� task
							_tasks.pop();
						}
						task();//ִ������
#ifdef THREADPOOL_AUTO_GROW
						if (_idlThrNum > 0 && _pool.size() > _initSize) //֧���Զ��ͷſ����߳�,�����ֵ������������߳�
							return;
#endif // !THREADPOOL_AUTO_GROW
						{
							std::unique_lock<std::mutex> lock{ _lock };
							_idlThrNum++;
						}
					}
					});
				{
					std::unique_lock<std::mutex> lock{ _lock };
					_idlThrNum++;
				}
			}
		}
	};

}

#endif  // THREAD_POOL_H