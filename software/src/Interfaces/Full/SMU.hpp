#pragma once

#include <DBCInterface.hpp>
#include <QObject>

namespace real {

class SMU : public QObject, public CAN::DBCInterface<SMU> {
    Q_OBJECT
  public:
    SMU(const std::string& dbc_file_path = "SMU_CANBUS.dbc") : QObject(nullptr), DBCInterface(dbc_file_path) {
        can_signal_dispatch[0x1838F380] = &SMU::handleGeneralBroadcast;
    }

  signals:
    void newThermistorTemp(int segment, int id, int temp);

  private:
    void handleGeneralBroadcast(const dbcppp::IMessage* message_decoder, const can_frame& frame) {
        auto sig = message_decoder["Thermistor_ID"];
        int thermistor_id = sig.RawToPhys(sig.Decode(frame.data));
        sig = message_decoder["Thermistor_Temperature"];
        int thermistor_temp = sig.RawToPhys(sig.Decode(frame.data));
        emit newThermistorTemp(thermistor_id / 80, thermistor_id % 80, thermistor_temp);
    }

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {{
                                                            0X1838F380,
                                                            0x7FFFFFFF // Grab 0X1838F380 for general broadcast messages
                                                        }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real
