#pragma once

#include "pch.h"

class ThreadPool
{
    // 内置线程工作类
    class ThreadWorker 
    {
    public:
        ThreadWorker(ThreadPool* pool, const unsigned int id) : m_pool(pool), m_id(id) { }

        void operator () ()
        {
            std::function<void()> func;
            bool success;

            while (!m_pool->m_shutdown)
            {
                {
                    std::unique_lock<std::mutex> lock(m_pool->m_conditionalMutex);

                    if (m_pool->m_queue.Empty()) // 如果任务队列为空，阻塞当前线程
                    {
                        m_pool->m_conditionalVariable.wait(lock);
                    }

                    success = m_pool->m_queue.Dequeue(func);
                }
                if (success) func();
            }
        }

    private:
        unsigned int m_id;          
        ThreadPool* m_pool; 
    };


public:
    // 线程池构造函数
    ThreadPool() = default;
    ~ThreadPool() = default;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;

    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

public:
    void Init(unsigned int threadCount)
    {
        for (unsigned int i = 0; i < threadCount; ++i)
        {
            m_threads.emplace_back(std::thread{ ThreadWorker(this, i) });
        }
    }

    void Shut()
    {
        m_shutdown = true;
        m_conditionalVariable.notify_all(); // 通知，唤醒所有工作线程，使其结束

        for (unsigned int i = 0; i < m_threads.size(); ++i)
        {
            if (m_threads[i].joinable()) m_threads[i].join();
        }
    }

    template <typename F, typename... Args>
    auto Submit(F&& f, Args &&...args) -> std::future<decltype(f(args...))>
    {
        // create a function with bounded parameter ready to execute
        std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...); 

        // encapsulate it into a shared pointer in order to be able to copy construct
        auto taskPtr = CreateRef<std::packaged_task<decltype(f(args...))()>>(func);

        // warp packaged task into void function
        std::function<void()> warpper = [taskPtr]()
        {
            (*taskPtr)();
        };

        m_queue.Enqueue(warpper);
        m_conditionalVariable.notify_one();

        return taskPtr->get_future();
    }

private:
    bool m_shutdown;

    SafeQueue<std::function<void()>> m_queue;
    std::vector<std::thread> m_threads;

    std::mutex m_conditionalMutex;                 // 线程休眠锁互斥变量
    std::condition_variable m_conditionalVariable; // 线程环境锁，可以让线程处于休眠或者唤醒状态
};