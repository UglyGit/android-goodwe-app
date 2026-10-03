cmake_minimum_required(VERSION 3.16)

project(goodwe-android-tv-monitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick SerialBus Charts Test)

set(PROJECT_SOURCES
    src/main.cpp
    src/inverter_config.h
    src/inverter_config.cpp
    src/modbus_client.h
    src/modbus_client.cpp
    src/telemetry_mapper.h
    src/telemetry_mapper.cpp
    src/polling_manager.h
    src/polling_manager.cpp
    src/app_state_orchestrator.h
    src/app_state_orchestrator.cpp
    src/history_repository.h
    src/history_repository.cpp
)

if(ANDROID)
    qt_add_executable(goodwe-android-app ${PROJECT_SOURCES})
    qt_add_qml_module(goodwe-android-app
        URI GoodWeApp
        VERSION 1.0
        QML_FILES src/MainForm.qml src/DashboardView.qml
    )
    target_include_directories(goodwe-android-app PRIVATE src)
    target_link_libraries(goodwe-android-app PRIVATE
        Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::SerialBus Qt6::Charts
    )
else()
    qt_add_executable(goodwe-desktop-app ${PROJECT_SOURCES})
    
    qt_add_qml_module(goodwe-desktop-app
        URI GoodWeApp
        VERSION 1.0
        QML_FILES src/MainForm.qml src/DashboardView.qml
    )
    
    target_include_directories(goodwe-desktop-app PRIVATE src)
    target_link_libraries(goodwe-desktop-app PRIVATE
        Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::SerialBus Qt6::Charts
    )

    # Unit Testing Infrastructure Framework linkage targets
    set(TEST_SOURCES
        tests/main_test.cpp
        src/inverter_config.h
        src/inverter_config.cpp
        src/modbus_client.h
        src/modbus_client.cpp
        src/telemetry_mapper.h
        src/telemetry_mapper.cpp
        src/polling_manager.h
        src/polling_manager.cpp
        src/app_state_orchestrator.h
        src/app_state_orchestrator.cpp
        src/history_repository.h
        src/history_repository.cpp
    )
    qt_add_executable(goodwe-test-runner ${TEST_SOURCES})
    target_include_directories(goodwe-test-runner PRIVATE src)
    
    target_link_libraries(goodwe-test-runner PRIVATE
        Qt6::Core
        Qt6::SerialBus
        Qt6::Test
    )
endif()
