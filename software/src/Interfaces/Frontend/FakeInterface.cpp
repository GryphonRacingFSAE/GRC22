#include <Frontend/FakeInterface.hpp>

using namespace fake;

void FakeInterface::startReceiving() {
    m_reading_thread = std::thread(&FakeInterface::readLoop, this);
}

void FakeInterface::readLoop() {
    while (!this->m_should_exit) {
        std::this_thread::sleep_for(this->delay);
        this->generateValues();
    }
    this->m_should_exit = false;
}

void FakeInterface::stopReceiving() {
    m_should_exit = true;
    if (m_reading_thread.joinable()) {
        m_reading_thread.join();
    }
}

FakeInterface::~FakeInterface() {
    this->stopReceiving();
}