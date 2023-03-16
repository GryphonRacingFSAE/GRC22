#pragma once

#include <atomic>
#include <thread>

namespace fake {

class FakeInterface {
  public:
    FakeInterface() = default;
    virtual ~FakeInterface();

    void startReceiving();

  protected:
    void stopReceiving();
    virtual void generateValues() = 0;

  private:
    void readLoop();

  private:
    std::thread m_reading_thread;
    std::atomic<bool> m_should_exit = false;

  protected:
    std::chrono::duration delay = std::chrono::milliseconds(100);
};

} // namespace CAN