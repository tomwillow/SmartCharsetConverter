#pragma once

#include <queue>
#include <mutex>
#include <optional>

template <typename T> class SafeQueue {
public:
    SafeQueue() {}

    bool empty() const {
        std::scoped_lock<std::mutex> sl(m);
        return q.empty();
    }

    std::size_t size() const {
        std::scoped_lock<std::mutex> sl(m);
        return q.size();
    }

    template <typename U> void enqueue(U &&t) {
        std::scoped_lock<std::mutex> sl(m);
        q.push(std::forward<U>(t));
    }

    std::optional<T> dequeue() {
        std::scoped_lock<std::mutex> sl(m);
        if (q.empty()) {
            return {};
        }
        T t = std::move(q.front());
        q.pop();
        return t;
    }

private:
    std::queue<T> q;
    mutable std::mutex m;
};