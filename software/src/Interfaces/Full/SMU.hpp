#pragma once

#include <DBCInterface.hpp>
#include <QObject>
#include <algorithm>

namespace real {

class SMU : public QObject, public CAN::DBCInterface<SMU> {
    Q_OBJECT
    Q_PROPERTY(QList<int> temperatures MEMBER m_temperatures NOTIFY temperaturesChanged)
    Q_PROPERTY(QList<bool> filtered MEMBER m_filtered NOTIFY filteredChanged)
  public:
    SMU(const std::string& dbc_file_path = "SMU_CANBUS.dbc")
        : QObject(nullptr), DBCInterface(dbc_file_path), m_temperatures(5 * 56, -40), m_filtered(5 * 56, false) {
        can_message_dispatch[0x1838F380] = &SMU::handleGeneralBroadcast;
        can_message_dispatch[0x183AF380] = &SMU::handleFilteredBroadcast;
    }

  signals:
    void newThermistorTemp(int segment, int id, int temp);
    void temperaturesChanged();
    void filteredChanged();

  private:
    void handleGeneralBroadcast(const dbcppp::IMessage* message_decoder, const can_frame& frame) {
        int thermistor_temp = -40, thermistor_id = -1;
        for (const dbcppp::ISignal& sig : message_decoder->Signals()) {
            if (sig.Name() == "Thermistor_ID") {
                thermistor_id = sig.RawToPhys(sig.Decode(frame.data));
            }
            if (sig.Name() == "Thermistor_Temperature") {
                thermistor_temp = sig.RawToPhys(sig.Decode(frame.data));
            }
        }

        // Only update if we actually have a thermistor ID
        if (thermistor_id != -1) {
            m_temperatures[thermistor_id / 80 * 56 + thermistor_id % 80] = thermistor_temp;
            emit newThermistorTemp(thermistor_id / 80, thermistor_id % 80, thermistor_temp);
        }

        // Only update all the temperatures once we've looped through each segment to conserve resources?
        if (thermistor_id % 80 == 0) {
            emit temperaturesChanged();
        }
    }
    void handleFilteredBroadcast(const dbcppp::IMessage* message_decoder, const can_frame& frame) {
        int thermistor_id = -1;
        bool is_filtered = false;
        for (const dbcppp::ISignal& sig : message_decoder->Signals()) {
            if (sig.Name() == "Thermistor_ID") {
                thermistor_id = sig.RawToPhys(sig.Decode(frame.data));
            }
            if (sig.Name() == "Filtered") {
                is_filtered = sig.RawToPhys(sig.Decode(frame.data));
            }
        }

        // Only update if we actually have a thermistor ID
        if (thermistor_id != -1) {
            if (is_filtered != m_filtered[thermistor_id / 80 * 56 + thermistor_id % 80]) {
                m_filtered[thermistor_id / 80 * 56 + thermistor_id % 80] = is_filtered;
                // emit newThermistorTemp(thermistor_id / 80, thermistor_id % 80, thermistor_temp);
                emit filteredChanged();
            }
        }
    }

  private:
    QList<int> m_temperatures;
    QList<bool> m_filtered;

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {{
        0X1838F380,
        0x1FFDFFFF // Grab 0X1838F380 and 0X183AF380 for general broadcast messages
    }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real
