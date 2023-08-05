#pragma once

#include "SafeQueue.h"

#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>

#undef min
#undef max

class ThreadPool {
public:
    ThreadPool() : doStop(false) {
        int workerNums = std::max(1, static_cast<int>(std::thread::hardware_concurrency()) - 1);
        for (int i = 0; i < workerNums; i++) {
            vecth.push_back(std::thread(&ThreadPool::workLoop, this));
        }
    }

    ~ThreadPool() {
        {
            std::scoped_lock sl(m);
            doStop = true;
        }

        cv.notify_all();

        for (auto &th : vecth) {
            th.join();
        }
    }

    template <typename F, typename... Args> auto submit(F &&fn, Args &&...args) -> std::future<decltype(fn(args...))> {
        std::function<decltype(fn(args...)())> fn = std::bind(std::forward(fn), std::forward(args)...);

        auto taskPtr = std::make_shared<std::packaged_task<decltype(fn(args...))()>>(fn);

        std::function<void()> wrapper = [taskPtr]() { (*taskPtr)(); };

        q.enqueue(wrapper);

        cv.notify_one();

        return taskPtr->get_future();
    }

private:
    std::vector<std::thread> vecth;
    bool doStop;

    SafeQueue<std::function<void()>> q;

    std::mutex m;
    std::condition_variable cv;

    void workLoop() {
        while (1) {
            std::unique_lock ul(m);
            while (!doStop || q.empty()) {
                cv.wait(ul);
            }

            if (doStop)
                return;

            auto t = q.dequeue();

            ul.unlock();

            if (!t.has_value())
                continue;

            auto fn = t.value();
            fn();
        }
    }
};