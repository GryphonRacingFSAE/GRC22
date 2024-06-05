if(NOT ${BUILD_FRONTEND})
    return()
endif()

add_library(CANInterfaces)

if(BUILD_BACKEND)
    target_sources(CANInterfaces PUBLIC Full/MotorController.hpp Full/EnergyMeter.hpp Full/VCU.hpp Full/BMS.hpp Full/SMU.hpp Full/FGCANLogger.hpp)

    target_link_libraries(CANInterfaces PUBLIC dbcppp::dbcppp CANLib FGLoggerLib)
endif()

set_target_properties(
    CANInterfaces
    PROPERTIES AUTOUIC ON
               AUTOMOC ON
               AUTORCC ON)
target_include_directories(CANInterfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CANInterfaces PUBLIC qt::qt fmt::fmt rapidcsv::rapidcsv)
