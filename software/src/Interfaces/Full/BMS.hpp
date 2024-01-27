#pragma once

#include <DBCInterface.hpp>
#include <QObject>

namespace real {

class BMS : public QObject, public CAN::DBCInterface<BMS> {
    Q_OBJECT
    Q_PROPERTY(QList<int> voltages MEMBER m_voltages NOTIFY voltagesChanged)
  public:
    BMS(const std::string& dbc_file_path = "Orion_CANBUS.dbc") : QObject(nullptr), DBCInterface(dbc_file_path) {
        can_signal_dispatch["Pack_Open_Voltage"] = &BMS::newAccumulatorOpenVoltage;
        can_signal_dispatch["Pack_SOC"] = &BMS::newAccumulatorSOC;
        can_signal_dispatch["Pack_Inst_Voltage"] = &BMS::newAccumulatorInstVoltage;
        can_signal_dispatch["Pack_Current"] = &BMS::newAccumulatorCurrent;
        can_signal_dispatch["High_Temperature"] = &BMS::newAccumulatorMaxTemp;
        can_signal_dispatch["Internal_Temperature"] = &BMS::newBMSTemp;
        can_message_dispatch[0X108] = &BMS::handleGeneralBroadcast;
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

  private void handleGeneralBroadcast(const dbcppp::IMessage* message_decoder, const can_frame& frame) {
    int voltage = -40m cell_id = -1;
    for(const dbcppp::ISignal& sig : message_decoder->Signals()){
      if(sig.Name() == "CellId"){
        cell_id = sig.RawToPhys(sig.Decode(frame.data));
      }
      if(sig.Name() == "CellVoltage"){
        voltage = sig.RawToPhys(sig.Decode(frame.data));
      }
    }

    if(cell_id != -1){
      //add to voltage arr
      m_voltages[cell_id] = voltage;
      emit newCellVoltage(cell_id / 26, cell_id % 26, voltage);
    }
  }

  private:
    QList<int> m_voltages;

  public:
    static constexpr size_t num_of_filters = 3;
    inline static can_filter filters[num_of_filters] = {{
        0X108, //general broadcast messages
        0x0E0, 
        0x7F0 // Grab 0x0E0 to 0x0EF for broadcast messages
    }};

    static constexpr uint32_t timeout_ms = 500;
};

} // namespace real
