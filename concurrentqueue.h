#ifndef CONCURRENTQUEUE_H
#define CONCURRENTQUEUE_H

#include <deque>
#include <condition_variable>

extern "C" {
    #include <pcap.h>
}

template <typename T> class ConcurrentQueue {
  private:
    std::deque<T> queue_;
    std::mutex mtx_;
    std::condition_variable queue_empty_cond_var_;
    std::condition_variable queue_full_cond_var_;
    size_t max_size_ = 0;
    bool stopping = false;

  public:
    ConcurrentQueue() {}
    ConcurrentQueue(size_t max_size) : max_size_(max_size) {}

    void push(const T &val) {
        {
            std::unique_lock<std::mutex> lck(mtx_);
            while (queue_.size() >= max_size_) {
                queue_full_cond_var_.wait(lck);
                if (stopping) {
                    return;
                }
            }
            //qInfo("Buffer push size: %i", queue_.size());
            queue_.push_back(val);
        }
        queue_empty_cond_var_.notify_one();
    }
    T pop() {
        T return_val;
        {
            std::unique_lock<std::mutex> lck(mtx_);
            while (queue_.empty()) {
                queue_empty_cond_var_.wait(lck);
                if (stopping) {
                    return T(); // supposed to be a stop object
                }
            }
            //qInfo("Buffer pop size: %i", queue_.size());
            return_val = queue_.front();
            queue_.pop_front();
        }
        queue_full_cond_var_.notify_one();
        return return_val;
    }
    // stop waiting and return back
    void stop() {
        {
            std::unique_lock<std::mutex> lck(mtx_);
            stopping = true;
        }
        queue_empty_cond_var_.notify_all();
        queue_full_cond_var_.notify_all();
    }
//    T peek() {
//        std::unique_lock<std::mutex> lck(mtx_);
//        while (queue_.empty()) {
//            queue_empty_cond_var_.wait(lck);
//        }
//        return queue_.front();
//    }
//    T peekBack() {
//        std::unique_lock<std::mutex> lck(mtx_);
//        while(queue_.empty()) {
//            queue_empty_cond_var_.wait(lck);
//        }
//        return queue_.back();
//    }
    //size_t size() const { return queue_.size(); }
    size_t max_size() const { return max_size_; }
};

#endif // CONCURRENTQUEUE_H
