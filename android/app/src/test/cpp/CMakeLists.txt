# CMakeLists.txt for Android C++ unit tests
# Builds Google Test-based tests for engine_sim_core components

cmake_minimum_required(VERSION 3.18)

project(engine_sim_android_test)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fno-rtti -fno-exceptions -fvisibility=hidden)

# ========================================================
# Google Test
# ========================================================
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

# ========================================================
# Simple 2D Constraint Solver (SCS)
# ========================================================
add_subdirectory(${CMAKE_SOURCE_DIR}/../../../core/third_party/simple-2d-constraint-solver
                 ${CMAKE_BINARY_DIR}/third_party/SCS)

# ========================================================
# Core Simulation Sources (shared with main build)
# ========================================================
set(ENGINE_SIM_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/simulator.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/piston_engine_simulator.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/engine.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/piston.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/crankshaft.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/cylinder_bank.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/connecting_rod.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/combustion_chamber.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/cylinder_head.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/exhaust_system.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/intake.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/fuel.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/ignition_module.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/valvetrain.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/standard_valvetrain.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/vtec_valvetrain.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/camshaft.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/throttle.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/transmission.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/vehicle.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/vehicle_drag_constraint.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/starter_motor.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/dynamometer.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/governor.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/gas_system.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/direct_throttle_linkage.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/part.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/synthesizer.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/impulse_response.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/low_pass_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/convolution_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/delay_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/feedback_comb_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/derivative_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/gaussian_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/jitter_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/leveling_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/audio_buffer.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/utilities.cpp
    ${CMAKE_SOURCE_DIR}/../../../core/src/engine_sim/function.cpp
)

# ========================================================
# Core static library
# ========================================================
add_library(engine_sim_core_test STATIC ${ENGINE_SIM_CORE_SOURCES})

target_include_directories(engine_sim_core_test PUBLIC
    ${CMAKE_SOURCE_DIR}/../../../core/include
    ${CMAKE_SOURCE_DIR}/../../../core/third_party/simple-2d-constraint-solver/include
)

target_link_libraries(engine_sim_core_test PRIVATE
    simple-2d-constraint-solver
)

set_target_properties(engine_sim_core_test PROPERTIES
    OUTPUT_NAME "engine_sim_core_test"
    POSITION_INDEPENDENT_CODE ON
)

# ========================================================
# Test executables
# ========================================================
add_executable(test_engine test_engine.cpp)
target_link_libraries(test_engine
    engine_sim_core_test
    gtest
    gtest_main
    simple-2d-constraint-solver
)
target_include_directories(test_engine PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../core/include
)

add_executable(test_synthesizer test_synthesizer.cpp)
target_link_libraries(test_synthesizer
    engine_sim_core_test
    gtest
    gtest_main
    simple-2d-constraint-solver
)
target_include_directories(test_synthesizer PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../core/include
)

add_executable(test_piston test_piston.cpp)
target_link_libraries(test_piston
    engine_sim_core_test
    gtest
    gtest_main
    simple-2d-constraint-solver
)
target_include_directories(test_piston PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../core/include
)

# ========================================================
# Register tests with CTest
# ========================================================
include(GoogleTest)
gtest_add_tests(TARGETS test_engine test_synthesizer test_piston)