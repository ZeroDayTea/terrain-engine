#pragma once

#include <mutex>
#include <condition_variable>
#include <queue>
#include <optional>

template<typename T>
class BlockingQueue {
    std::mutex m;
    std::condition_variable cv;
    std::queue<T> q;
    bool closed = false;
public:
    void push(T v) {
        std::lock_guard<std::mutex> lk(m);
        if (closed) {
            return;
        }
        q.push(std::move(v));
        cv.notify_one();
    }
    std::optional<T> pop() {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [&]{ return closed || !q.empty(); });
        if (q.empty()) {
            return std::nullopt;
        }
        T v = std::move(q.front());
        q.pop();
        return v;
    }
    void close() {
        std::lock_guard<std::mutex> lk(m);
        closed = true;
        cv.notify_all();
    }
};

template<typename T>
class SPSCQueue {
    std::mutex m;
    std::queue<T> q;
public:
    void push(T v) {
        std::lock_guard<std::mutex> lk(m);
        q.push(std::move(v));
    }
    bool try_pop(T& out) {
        std::lock_guard<std::mutex> lk(m);
        if (q.empty()) {
            return false;
        }
        out = std::move(q.front());
        q.pop();
        return true;
    }
};
