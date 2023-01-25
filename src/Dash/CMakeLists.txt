add_executable(GryphonDash main.cpp qml.qrc)
add_executable(GryphonDash2019 Dash2019.cpp qml.qrc)
set_target_properties(GryphonDash PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
set_target_properties(GryphonDash2019 PROPERTIES AUTOUIC ON AUTOMOC ON AUTORCC ON)
target_compile_definitions(GryphonDash PRIVATE ROOT_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(GryphonDash2019 PRIVATE ROOT_SOURCE_PATH="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(GryphonDash PRIVATE fmt::fmt qt::qt runtimeqml::runtimeqml CANInterfaces)
target_link_libraries(GryphonDash2019 PRIVATE fmt::fmt qt::qt runtimeqml::runtimeqml CANInterfaces)

# TODO: Add the deployment for linux
if(WIN32)
    add_custom_command(
        TARGET GryphonDash POST_BUILD
        COMMAND windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:GryphonDash>
    )
        
    add_custom_command(
        TARGET GryphonDash2019 POST_BUILD
        COMMAND windeployqt.exe --qmldir ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE_DIR:GryphonDash2019>
    )
elseif(UNIX AND NOT APPLE)
    # Package into AppImages only on Release, and the LINUX_X86_64_BUILD_APP_IMAGE env var is defined
    if(DEFINED ENV{LINUX_X86_64_BUILD_APP_IMAGE} AND CMAKE_BUILD_TYPE STREQUAL Release)  
        file(GLOB dbc_files ${CMAKE_SOURCE_DIR}/src/Interfaces/Full/*.dbc)
        add_custom_command(
            TARGET GryphonDash POST_BUILD
            DEPENDS GryphonDash.desktop images/GryphonDash.png
            COMMAND rm -r -f $<TARGET_FILE_DIR:GryphonDash>/$<TARGET_FILE_BASE_NAME:GryphonDash>AppDir
            COMMAND mkdir -p $<TARGET_FILE_DIR:GryphonDash>/$<TARGET_FILE_BASE_NAME:GryphonDash>AppDir/usr/bin
            COMMAND cp ${dbc_files} -t $<TARGET_FILE_DIR:GryphonDash>/$<TARGET_FILE_BASE_NAME:GryphonDash>AppDir/usr/bin
            COMMAND ${CMAKE_SOURCE_DIR}/linuxdeploy-x86_64.AppImage
                    -v2
                    --appdir $<TARGET_FILE_DIR:GryphonDash>/$<TARGET_FILE_BASE_NAME:GryphonDash>AppDir
                    --executable $<TARGET_FILE:GryphonDash> 
                    --icon-file ${CMAKE_CURRENT_SOURCE_DIR}/images/GryphonDash.png
                    --desktop-file ${CMAKE_CURRENT_SOURCE_DIR}/GryphonDash.desktop
                    --plugin qt
                    --output appimage
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin/2022
            COMMAND_EXPAND_LISTS
        )
    else()
        message(STATUS "No deployment for Linux, implement.")
    endif()
elseif(APPLE)
    message(STATUS "No deployment for Apple, implement.")
else()
    message(STATUS "No deployment for unknown platform, implement.")
endif()