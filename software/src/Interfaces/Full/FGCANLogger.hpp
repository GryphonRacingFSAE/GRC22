#pragma once

#include <FGLogger.hpp>
#include <Interface.hpp>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>

namespace real {

class FGCANLogger : public CAN::Interface {
  public:
    FGCANLogger(std::string dbc_folder = "DBCs")
        : logger(dbc_folder, "protos.desc", 8765, "/media/grc/ESD-USB/telemetry"), saving_thread(&FGCANLogger::loggingThread, this) {
        this->CAN::Interface::startReceiving("can0", nullptr, 0, FGCANLogger::timeout_ms); // Get everything
    }
    virtual ~FGCANLogger() {
        this->CAN::Interface::stopReceiving();
    }

  private:
    // As of now, newFrame and newError should be extremely fast functions before emitting more
    // signals
    void newFrame(const can_frame& frame) {
        uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        std::lock_guard<std::mutex> lock(queue_mutex);
        // Drop data if we build up a queue for whatever reason
        if (frame_queue.size() >= max_queue_size) {
            frame_queue.pop();
            timestamp_queue.pop();
        }
        frame_queue.push(frame);
        timestamp_queue.push(timestamp);
        queue_signaler.notify_one();
    }
    void newError(const can_frame&){};
    void newTimeout(){};

    void loggingThread() {
        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutex);
            queue_signaler.wait(lock, [this]() { return !frame_queue.empty(); });

            // retrieve item
            can_frame frame = frame_queue.front();
            frame_queue.pop();
            uint64_t timestamp = timestamp_queue.front();
            timestamp_queue.pop();
            lock.unlock();
            logger.saveAndPublish(frame, timestamp);
        }
    }

  private:
    static constexpr uint32_t timeout_ms = 500;
    static constexpr uint32_t max_queue_size = 10000;
    FGLogger logger;
    std::thread saving_thread;
    std::queue<can_frame> frame_queue;
    std::queue<uint64_t> timestamp_queue;
    std::mutex queue_mutex;
    std::condition_variable queue_signaler;
};

} // namespace real
