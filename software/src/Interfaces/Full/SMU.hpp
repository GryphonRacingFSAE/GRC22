#pragma once

#include <DBCInterface.hpp>
#include <QObject>
#include <algorithm>

namespace real {

class SMU : public QObject, public CAN::DBCInterface<SMU> {
    Q_OBJECT
    Q_PROPERTY(QList<int> temperatures MEMBER m_temperatures NOTIFY temperaturesChanged)
  public:
    SMU(const std::string& dbc_file_path = "SMU_CANBUS.dbc") : QObject(nullptr), DBCInterface(dbc_file_path), m_temperatures(5 * 56, -40) {
        can_message_dispatch[0x1838F380] = &SMU::handleGeneralBroadcast;
    }

  signals:
    void newThermistorTemp(int segment, int id, int temp);
    void temperaturesChanged();

  private:
    void handleGeneralBroadcast(const dbcppp::IMessage* message_decoder, const can_frame& frame) {
        int thermistor_temp, thermistor_id = 0;
        for (const dbcppp::ISignal& sig : message_decoder->Signals()) {
            if (sig.Name() == "Thermistor_ID") {
                thermistor_temp = sig.RawToPhys(sig.Decode(frame.data));
            }
            if (sig.Name() == "Thermistor_Temperature") {
                thermistor_id = sig.RawToPhys(sig.Decode(frame.data));
            }
        }

        m_temperatures[thermistor_id / 80 * 56 + thermistor_id % 80] = thermistor_temp;

        // Only update all the temperatures once we've looped through everything to conserve resources?
        if (thermistor_id == 0) {
            emit temperaturesChanged();
        }
        emit newThermistorTemp(thermistor_id / 80, thermistor_id % 80, thermistor_temp);
    }

  private:
    QList<int> m_temperatures;

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {{
        0X1838F380,
        0x7FFFFFFF // Grab 0X1838F380 for general broadcast messages
    }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real
