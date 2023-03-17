if (NOT ${BUILD_FRONTEND})
    return()
endif()

add_library(CANInterfaces)
target_sources(CANInterfaces PUBLIC
    Full/MotorController.hpp
    Full/EnergyMeter.hpp
    Full/VCU.hpp
    Full/BMS.hpp
    Demo/MotorController.hpp
    Frontend/FakeInterface.hpp
    Frontend/FakeInterface.cpp
    Frontend/MotorController.hpp
    Frontend/MotorController.cpp
    Frontend/EnergyMeter.cpp
    Frontend/EnergyMeter.hpp
    Frontend/BMS.hpp
    Frontend/BMS.cpp
    Frontend/VCU.hpp
)
set_target_properties(CANInterfaces PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
target_include_directories(CANInterfaces PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CANInterfaces PUBLIC qt::qt fmt::fmt rapidcsv::rapidcsv CANLib dbcppp::dbcppp)