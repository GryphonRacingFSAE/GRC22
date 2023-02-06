#include <cassert>
#include <tools.hpp>
#include <fmt/core.h>


// Mask for can errors
bool CAN::isError(const can_frame& frame) {
    return frame.can_id & CAN_ERR_FLAG;
}

// Mask for RTRs
bool CAN::isRemoteTransmissionRequest(const can_frame& frame) {
    return frame.can_id & CAN_RTR_FLAG;
}

// Determines the format of the CAN frame
CanFormat CAN::frameFormat(const can_frame& frame) {
    if (frame.can_id & CAN_EFF_FLAG) {
        return CanFormat::Extended;
    }
    return CanFormat::Standard;
}


// Recieve and emit data for the Debugger screen
void CAN::emitDebugger(const can_frame& frame) {
    unsigned int a = 0xC0;
    unsigned int relayCommand = 0x01;
    unsigned int fluxCommand = 0x10;
    unsigned int resolverCommand = 0x11;
    unsigned int gammaAdjustCommand = 0x12;
    unsigned int faultClearCommand = 0x20;
    unsigned int SetPWMFrequencyCommand = 0x21;
    unsigned int AINPullupCommand = 0x22;
    unsigned int shudderCompensationCommand = 0x23;
    unsigned int DiagnosticDataTriggerCommand = 0x31;

    if (frame.can_id == a) {
        emit frame.data[0] && frame.data[1] //Torque command
        emit frame.data[2] && frame.data[3] //Speed command
        emit frame.data[4] //Direction Command

        byte = frame.data[5]
        emit ((byte >> 0) & 0x00) //Inverter Enable
        emit ((byte >> 1) & 0x01) //Inverter Discharge
        emit ((byte >> 2) & 0x02) //Speed Mode Enable

        emit frame.data[6] && frame.data[7] //Commanded Torque Limit
    }

    if (frame.can_id == relayCommand) {
        emit frame.data //Relay Command
    }

    if (frame.can_id == fluxCommand) {
        emit frame.data
    }

    if (frame.can_id == resolverCommand) {
        emit frame.data
    }

    if (frame.can_id == gammaAdjustCommand) {
        emit frame.data
    }

    if (frame.can_id == faultClearCommand) {
        emit frame.data
    }

    if (frame.can_id == SetPWMFrequencyCommand) {
        emit frame.data
    }

    if (frame.can_id == AINPullupCommand) {
        emit frame.data
    }

    if (frame.can_id == shudderCompensationCommand) {
        emit frame.data
    }

    if (frame.can_id == DiagnosticDataTriggerCommand) {
        emit frame.data
    }
}


// Masks for the CAN frame ID
canid_t CAN::frameId(const can_frame& frame) {
    switch (frameFormat(frame)) {
    case CanFormat::Standard:
        return frame.can_id & CAN_SFF_MASK; // 11 LSB
    case CanFormat::Extended:
        return frame.can_id & CAN_EFF_MASK; // 29 LSB
    default:
        break;
    }
    assert(false); // Only 2 formats supported
    return 0;
}