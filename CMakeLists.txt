cmake_minimum_required(VERSION 3.16)
project(SimTrrafic VERSION 1.3.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Threads REQUIRED)

add_library(simtrrafic_core src/Simulador.cpp)
target_include_directories(simtrrafic_core PUBLIC include)
target_link_libraries(simtrrafic_core PUBLIC Threads::Threads)

add_executable(simtrrafic src/main.cpp)
target_link_libraries(simtrrafic PRIVATE simtrrafic_core)

include(CTest)
if(BUILD_TESTING)
    add_executable(test_concurrencia tests/test_concurrencia.cpp)
    target_include_directories(test_concurrencia PRIVATE include)
    target_link_libraries(test_concurrencia PRIVATE Threads::Threads)
    add_test(NAME concurrencia_basica COMMAND test_concurrencia)

    add_executable(test_interseccion tests/test_interseccion.cpp)
    target_include_directories(test_interseccion PRIVATE include)
    target_link_libraries(test_interseccion PRIVATE Threads::Threads)
    add_test(NAME interseccion_segura COMMAND test_interseccion)
endif()
if(BUILD_TESTING)
    foreach(nombre configuracion fabrica cruce_seguro)
        add_executable(test_${nombre} tests/test_${nombre}.cpp)
        target_include_directories(test_${nombre} PRIVATE include)
        target_link_libraries(test_${nombre} PRIVATE Threads::Threads)
        add_test(NAME prueba_${nombre} COMMAND test_${nombre})
    endforeach()
endif()

if(BUILD_TESTING)
    add_executable(test_politica tests/test_politica.cpp)
    target_include_directories(test_politica PRIVATE include)
    add_test(NAME politica_adaptativa COMMAND test_politica)
endif()

if(BUILD_TESTING)
    add_executable(test_simulador tests/test_simulador.cpp)
    target_link_libraries(test_simulador PRIVATE simtrrafic_core)
    add_test(NAME simulacion_y_cancelacion COMMAND test_simulador)
endif()

if(BUILD_TESTING)
    add_executable(test_monitor tests/test_monitor.cpp)
    target_include_directories(test_monitor PRIVATE include)
    add_test(NAME monitor_json COMMAND test_monitor)
endif()

# Qt Quick es la interfaz principal. La consola no necesita Qt.
option(BUILD_QT_QUICK "Compilar la interfaz Qt Quick/QML" ON)
if(BUILD_QT_QUICK)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2)
    add_library(simtrrafic_qt_controller STATIC
        gui/SimulationController.cpp gui/SimulationController.hpp
        gui/VehicleModel.cpp gui/VehicleModel.hpp)
    set_target_properties(simtrrafic_qt_controller PROPERTIES AUTOMOC ON)
    target_include_directories(simtrrafic_qt_controller PUBLIC gui)
    target_link_libraries(simtrrafic_qt_controller PUBLIC simtrrafic_core Qt6::Core)

    qt_add_executable(simtrrafic_qt gui/main_quick.cpp)
    target_link_libraries(simtrrafic_qt PRIVATE simtrrafic_qt_controller
        Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2)
    # Recursos locales: no se necesita copiar los QML junto al ejecutable.
    qt_add_resources(simtrrafic_qt "simtrrafic_qml"
        PREFIX "/simtrrafic" BASE "${CMAKE_CURRENT_SOURCE_DIR}/gui"
        FILES gui/qml/Main.qml gui/qml/Theme.qml
            gui/qml/components/TopBar.qml gui/qml/components/ControlPanel.qml
            gui/qml/components/StatisticsPanel.qml gui/qml/components/EventLog.qml
            gui/qml/components/TrafficScene.qml gui/qml/components/Vehicle.qml
            gui/qml/components/TrafficLight.qml gui/qml/components/UrbanBlock.qml
            gui/qml/components/UiButton.qml gui/qml/components/UiSpinBox.qml
            gui/qml/components/UiComboBox.qml gui/qml/components/UiCheckBox.qml
            gui/qml/components/UiProgressBar.qml)
    if(BUILD_TESTING)
        add_executable(test_vehicle_model tests/test_vehicle_model.cpp)
        target_link_libraries(test_vehicle_model PRIVATE simtrrafic_qt_controller)
        add_test(NAME vehiculos_qt_quick COMMAND test_vehicle_model)
        find_package(Qt6 6.2 REQUIRED COMPONENTS Test)
        add_executable(test_qt_interface tests/test_qt_interface.cpp)
        target_link_libraries(test_qt_interface PRIVATE simtrrafic_qt_controller Qt6::Gui Qt6::Qml Qt6::Quick Qt6::QuickControls2 Qt6::Test)
        target_compile_definitions(test_qt_interface PRIVATE SIMTRAFFIC_QML_DIR="${CMAKE_CURRENT_SOURCE_DIR}/gui/qml")
        add_test(NAME interfaz_qt_quick_integracion COMMAND test_qt_interface)
        set_tests_properties(interfaz_qt_quick_integracion PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT 120)
        add_test(NAME interfaz_qt_quick_hidpi COMMAND test_qt_interface --layout-only)
        set_tests_properties(interfaz_qt_quick_hidpi PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software;QT_SCALE_FACTOR=1.5" TIMEOUT 20)
        add_executable(test_qt_controller tests/test_qt_controller.cpp)
        target_link_libraries(test_qt_controller PRIVATE simtrrafic_qt_controller)
        add_test(NAME controlador_qt_quick COMMAND test_qt_controller)
        set_tests_properties(controlador_qt_quick PROPERTIES TIMEOUT 30)
        add_test(NAME interfaz_qt_quick_smoke COMMAND simtrrafic_qt --smoke-test)
        set_tests_properties(interfaz_qt_quick_smoke PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software" TIMEOUT 15)
    endif()
endif()
