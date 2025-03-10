#ifndef UTIL_H
#define UTIL_H

#pragma once

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <iostream>
#include <assert.h>

#include <queue>
#include <mutex>
#include <condition_variable>

#define MAX_CAPACITY 20
/*
static uint64_t get_cur_time(void) {

    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    return (tv.tv_sec * 1000ULL) + (tv.tv_usec / 1000);

}
*/

namespace unifuzz {

    template<typename T>
    class BlockingQueue {
    public:
        BlockingQueue()
                : mtx(), full_(), empty_(), capacity_(MAX_CAPACITY) {}


        void Push(const T &data) {
            std::unique_lock<std::mutex> lock(mtx);
            while (queue_.size() == capacity_) {
                full_.wait(lock);
            }

            assert(queue_.size() < capacity_);
            queue_.push(data);
            empty_.notify_all();
        }

        T Pop() {
            std::unique_lock<std::mutex> lock(mtx);
            while (queue_.empty()) {
                empty_.wait(lock);
            }

            assert(!queue_.empty());
            T front(queue_.front());
            queue_.pop();
            full_.notify_all();
            return front;
        }

        T Front() {
            std::unique_lock<std::mutex> lock(mtx);
            while (queue_.empty()) {
                empty_.wait(lock);
            }

            assert(!queue_.empty());
            T front(queue_.front());
            return front;
        }

        T Back() {
            std::unique_lock<std::mutex> lock(mtx);
            while (queue_.empty()) {
                empty_.wait(lock);
            }

            assert(!queue_.empty());
            T back(queue_.back());
            return back;
        }

        size_t Size() {
            std::lock_guard<std::mutex> lock(mtx);
            return queue_.size();
        }

        bool Empty() {
            std::unique_lock<std::mutex> lock(mtx);
            return queue_.empty();
        }

        void SetCapacity(const size_t capacity) {
            capacity_ = (capacity > 0 ? capacity : MAX_CAPACITY);
        }

    private:
        //DISABLE_COPY_AND_ASSIGN(BlockingQueue);
        BlockingQueue(const BlockingQueue &rhs);

        BlockingQueue &operator=(const BlockingQueue &rhs);

    private:
        mutable std::mutex mtx;
        std::condition_variable full_;
        std::condition_variable empty_;
        std::queue<T> queue_;
        size_t capacity_;
    };


}// end namespace



#endif
