cmake_minimum_required(VERSION 3.16)
project(SimTrrafic VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# La interfaz utiliza el navegador como vista; no requiere bibliotecas GUI externas.
add_library(simtrrafic_web src/ServidorWeb.cpp)
target_include_directories(simtrrafic_web PUBLIC include)
target_link_libraries(simtrrafic_web PUBLIC simtrrafic_core Threads::Threads)
target_compile_definitions(simtrrafic_web PRIVATE SIMTRAFFIC_WEB_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/web")
if(WIN32)
    target_link_libraries(simtrrafic_web PRIVATE ws2_32)
endif()
target_link_libraries(simtrrafic PRIVATE simtrrafic_web)

find_package(Python3 COMPONENTS Interpreter QUIET)
if(BUILD_TESTING AND Python3_Interpreter_FOUND)
    add_test(NAME api_web_e2e
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_web_api.py $<TARGET_FILE:simtrrafic>)
    set_tests_properties(api_web_e2e PROPERTIES TIMEOUT 20)
endif()

# Interfaz de escritorio opcional en Qt 6 Widgets (independiente del panel web).
option(BUILD_QT_GUI "Compilar la aplicacion de escritorio Qt 6" OFF)
if(BUILD_QT_GUI)
    find_package(Qt6 6.2 REQUIRED COMPONENTS Widgets)
    add_executable(simtrrafic_qt
        gui/main_qt.cpp
        gui/MainWindow.cpp
        gui/TrafficCanvas.cpp
    )
    target_include_directories(simtrrafic_qt PRIVATE gui include)
    target_link_libraries(simtrrafic_qt PRIVATE simtrrafic_core Qt6::Widgets)

    # Prueba de apertura de la ventana sin necesidad de un monitor fisico.
    if(BUILD_TESTING)
        add_test(NAME interfaz_qt_smoke COMMAND $<TARGET_FILE:simtrrafic_qt> --smoke-test)
        set_tests_properties(interfaz_qt_smoke PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen"
            TIMEOUT 15
        )
    endif()
endif()
