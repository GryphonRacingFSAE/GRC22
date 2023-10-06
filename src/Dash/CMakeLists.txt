add_executable(GryphonDash main.cpp qml.qrc)
add_executable(GryphonDashDemo DemoDash.cpp qml.qrc)
set_target_properties(GryphonDash PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
set_target_properties(GryphonDashDemo PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
target_compile_definitions(GryphonDash PRIVATE ROOT_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(GryphonDashDemo PRIVATE ROOT_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(GryphonDash PRIVATE fmt::fmt qt::qt runtimeqml::runtimeqml CANInterfaces)
target_link_libraries(GryphonDashDemo PRIVATE fmt::fmt qt::qt runtimeqml::runtimeqml CANInterfaces)

add_custom_command(
    TARGET GryphonDashDemo PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Interfaces/Full/DBCs
        $<TARGET_FILE_DIR:GryphonDashDemo>/DBCs
)
add_custom_command(
    TARGET GryphonDashDemo PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Interfaces/Full/TorqueMaps
        $<TARGET_FILE_DIR:GryphonDashDemo>/TorqueMaps
)

add_custom_command(
    TARGET GryphonDash PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Interfaces/Full/DBCs
        $<TARGET_FILE_DIR:GryphonDash>/DBCs
)
add_custom_command(
    TARGET GryphonDash PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/Interfaces/Full/TorqueMaps
        $<TARGET_FILE_DIR:GryphonDash>/TorqueMaps
)

if(WIN32)
    set_target_properties(GryphonDash PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/2022/$<0:>)
    set_target_properties(GryphonDash2019 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/2019/$<0:>)
    add_custom_command(
        TARGET GryphonDash POST_BUILD
        COMMAND windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:GryphonDash>
    )
        
    add_custom_command(
        TARGET GryphonDash2019 POST_BUILD
        COMMAND windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:GryphonDash2019>
    )
elseif(UNIX AND NOT APPLE)
    set_target_properties(GryphonDash PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/2022/$<0:>)
    set_target_properties(GryphonDash2019 PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/2019/$<0:>)
    file(GLOB dbc_files ${CMAKE_SOURCE_DIR}/src/Interfaces/Full/*.dbc)
    file(COPY ${dbc_files} DESTINATION $<TARGET_FILE_DIR:GryphonDash>)
    file(COPY ${dbc_files} DESTINATION $<TARGET_FILE_DIR:GryphonDash2019>)
elseif(APPLE)
    message(STATUS "No deployment for Apple, implement.")
else()
    message(STATUS "No deployment for unknown platform, implement.")
endif()
