#pragma once

#include <FGLogger.hpp>
#include <Interface.hpp>

namespace CAN::Interfaces {

class FGCANLogger : public Interface {
  public:
    FGCANLogger(std::string dbc_folder = "DBCs") : logger(dbc_folder, "protos.desc", 8765, ".") {
        this->Interface::startReceiving("can0", nullptr, 0, Dump::timeout_ms); // Get everything
    }
    virtual ~FGCANLogger() { 
        this->Interface::stopReceiving();
    }

  private:
    // As of now, newFrame and newError should be extremely fast functions before emitting more
    // signals
    void newFrame(const can_frame&) override {
        logger.saveAndPublish(frame);
    }
    void newError(const can_frame&) override;
    void newTimeout() override;

  private:
    static constexpr uint32_t timeout_ms = 500;
    FGLogger logger;
};

} // namespace CAN::Interfaces

#include <fmt/core.h>
#include <tools.hpp>

using namespace CAN::Interfaces;


void Dump::newFrame(const can_frame& frame) {

    fmt::print("ID: 0x{:08X}, Ext: {}, RTR: {}, Err: {}, Payload: 0x{:02X} 0x{:02X} 0x{:02X} "
               "0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X} 0x{:02X}\n",
               CAN::frameId(frame),
               CAN::frameFormat(frame) == CanFormat::Extended,
               CAN::isRemoteTransmissionRequest(frame),
               CAN::isError(frame),
               frame.data[0],
               frame.data[1],
               frame.data[2],
               frame.data[3],
               frame.data[4],
               frame.data[5],
               frame.data[6],
               frame.data[7]);
}

void Dump::newError(const can_frame&) {
    fmt::print("Error\n");
}

void Dump::newTimeout() {
    fmt::print("Timeout\n");
}