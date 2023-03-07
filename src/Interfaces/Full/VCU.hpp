#pragma once

#include <DBCInterface.hpp>
#include <QObject>

class VCU : public QObject, public CAN::DBCInterface<VCU> {
    Q_OBJECT
  public:
    VCU(const std::string& dbc_file_path = "VCU.dbc", const std::string& torque_map_directory = "")
        : QObject(nullptr), DBCInterface(dbc_file_path), m_torque_map_directory(torque_map_directory) {
    }

    Q_INVOKABLE void sendTorqueMap(QList<QList<int>> torque_map){
        (void) torque_map;
    }


  private:
    std::string m_torque_map_directory;

  public:
    static constexpr size_t num_of_filters = 1;
    inline static can_filter filters[num_of_filters] = {
        {
            0x0D0,
            0x0DF // Grab all messages from 0D0 to 0EF (32 Addresses)
        }};

    static constexpr uint32_t timeout_ms = 500;
};
