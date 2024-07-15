add_executable(GryphonDash qml.qrc)
if(BUILD_BACKEND)
    target_sources(GryphonDash PUBLIC main.cpp)
else()
    target_sources(GryphonDash PUBLIC fake.cpp)
endif()

set_target_properties(
    GryphonDash
    PROPERTIES AUTOUIC ON
               AUTOMOC ON
               AUTORCC ON)
target_compile_definitions(GryphonDash PRIVATE ROOT_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(GryphonDash PUBLIC fmt::fmt qt::qt runtimeqml::runtimeqml CANInterfaces)

add_custom_command(
    TARGET GryphonDash
    PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/DBCs $<TARGET_FILE_DIR:GryphonDash>/DBCs)

if(WIN32)
    set_target_properties(GryphonDash PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<0:>)
    message(STATUS " ${QT_PATH}/windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:GryphonDash>")
    add_custom_command(
        TARGET GryphonDash
        POST_BUILD
        COMMAND ${QT_PATH}/windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:GryphonDash>)
elseif(UNIX AND NOT APPLE)
    set_target_properties(GryphonDash PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/$<0:>)
    file(GLOB dbc_files ${CMAKE_SOURCE_DIR}/DBCs/*.dbc)
    file(COPY ${dbc_files} DESTINATION $<TARGET_FILE_DIR:GryphonDash>)
elseif(APPLE)
    message(STATUS "No deployment for Apple, implement.")
else()
    message(STATUS "No deployment for unknown platform, implement.")
endif()
