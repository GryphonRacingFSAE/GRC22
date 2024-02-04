#pragma once

#include <DBCInterface.hpp>
#include <QObject>

namespace real {

class BMS : public QObject, public CAN::DBCInterface<BMS> {
    Q_OBJECT
    Q_PROPERTY(QList<float> voltages MEMBER m_voltages NOTIFY voltagesChanged)
    Q_PROPERTY(QList<float> resistances MEMBER m_resistances NOTIFY resistancesChanged)
  public:
    BMS(const std::string& dbc_file_path = "Orion_CANBUS.dbc")
        : QObject(nullptr), DBCInterface(dbc_file_path), m_voltages(5 * 28, -40), m_resistances(5 * 28, -40) {
        can_signal_dispatch["Pack_Open_Voltage"] = &BMS::newAccumulatorOpenVoltage;
        can_signal_dispatch["Pack_SOC"] = &BMS::newAccumulatorSOC;
        can_signal_dispatch["Pack_Inst_Voltage"] = &BMS::newAccumulatorInstVoltage;
        can_signal_dispatch["Pack_Current"] = &BMS::newAccumulatorCurrent;
        can_signal_dispatch["High_Temperature"] = &BMS::newAccumulatorMaxTemp;
        can_signal_dispatch["Internal_Temperature"] = &BMS::newBMSTemp;
        can_message_dispatch[0x108] = &BMS::handleCellBroadcast;
    }

  signals:
    void newBMSTemp(float temp);
    void newAccumulatorMaxTemp(float temp);
    void newAccumulatorCurrent(float current);
    void newAccumulatorInstVoltage(float voltage);
    void newAccumulatorOpenVoltage(float voltage);
    void newAccumulatorSOC(float percent);

    void newCellVoltage(int segment, int id, float voltage);
    void voltagesChanged();

    void newCellResistance(int segment, int id, float ohms);
    void resistancesChanged();

  private:
    void handleCellBroadcast(const dbcppp::IMessage* message_decoder, const can_frame& frame) {
        float voltage = -40, resistance = -40;
        int cell_id = -1;
        for (const dbcppp::ISignal& sig : message_decoder->Signals()) {
            if (sig.Name() == "CellId") {
                cell_id = sig.RawToPhys(sig.Decode(frame.data));
            }
            if (sig.Name() == "CellVoltage") {
                voltage = sig.RawToPhys(sig.Decode(frame.data));
            }
            if (sig.Name() == "CellResistance") {
                resistance = sig.RawToPhys(sig.Decode(frame.data));
            }
        }

        if (cell_id != -1) {
            // Segment # is tied to which connector the segment is connected to on the BMS.
            // Each connector on the BMS is 36 cells wide (3 cell groups) but our configuration
            // has the top 2 cell groups partially populated, which produces weird cell IDs.
            // Cell ID 1-12 = Cells 1-12
            // Cell ID 13-20 = Cells 13-20
            // Cell ID 25-32 = Cell 21-28
            // Cell ID 21-24 and 33-36 are unpopulated
            // NOTE: the cell_id that we receive from the Orion is 0-indexed

            int segment_relative_cell_id = cell_id % 36;
            int segment_relative_cell_number = -1;
            if (segment_relative_cell_id < 20) {
                segment_relative_cell_number = segment_relative_cell_id;
            } else if (24 <= segment_relative_cell_id && segment_relative_cell_id < 32) {
                segment_relative_cell_number = segment_relative_cell_id - 4;
            } else {
                fmt::print("UNUSUAL CELL CONFIGURATION, POSSIBLY DANGEROUS! Detected Cell ID: {}!\n", cell_id);
                return;
            }

            // fmt::print("Segment #{}, Cell ID: {}\n", cell_idsegment_relative_cell_number, )

            // add to voltage arr
            m_voltages[cell_id / 36 * 28 + segment_relative_cell_number] = voltage;
            m_resistances[cell_id / 36 * 28 + segment_relative_cell_number] = resistance;

            emit newCellVoltage(cell_id / 36, segment_relative_cell_number, voltage);
            emit newCellResistance(cell_id / 36, segment_relative_cell_number, resistance);
        }

        if (cell_id == 0) {
            emit voltagesChanged();
            emit resistancesChanged();
        }
    }

  private:
    QList<float> m_voltages;
    QList<float> m_resistances;

  public:
    static constexpr size_t num_of_filters = 2;
    inline static can_filter filters[num_of_filters] = {{
                                                            0x0E0,
                                                            0x7F0 // Grab 0x0E0 to 0x0EF for broadcast messages
                                                        },
                                                        {
                                                            0x108,
                                                            0x7FF // grab general broadcast messages
                                                        }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real
