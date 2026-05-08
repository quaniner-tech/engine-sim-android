# CMakeLists.txt for Android app - builds JNI bridge + links core static lib
cmake_minimum_required(VERSION 3.18)

project(engine_sim_jni)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fno-rtti -fvisibility=hidden)

# =============================================================================
# Build engine_sim_core static library from core/
# =============================================================================
set(ENGINE_SIM_CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/simulator.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/piston_engine_simulator.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/engine.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/piston.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/crankshaft.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/cylinder_bank.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/connecting_rod.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/combustion_chamber.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/cylinder_head.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/exhaust_system.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/intake.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/fuel.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/ignition_module.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/valvetrain.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/standard_valvetrain.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/vtec_valvetrain.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/camshaft.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/throttle.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/transmission.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/vehicle.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/vehicle_drag_constraint.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/starter_motor.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/dynamometer.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/governor.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/gas_system.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/direct_throttle_linkage.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/part.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/synthesizer.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/impulse_response.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/low_pass_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/convolution_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/delay_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/feedback_comb_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/derivative_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/gaussian_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/jitter_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/leveling_filter.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/audio_buffer.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/utilities.cpp
    ${CMAKE_SOURCE_DIR}/../../../../../core/src/engine_sim/function.cpp
)

# SCS subdirectory
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../../../../core/third_party/simple-2d-constraint-solver
                 ${CMAKE_BINARY_DIR}/third_party/SCS)

# Build core static library
add_library(engine_sim_core STATIC ${ENGINE_SIM_CORE_SOURCES})

target_include_directories(engine_sim_core PUBLIC
    ${CMAKE_SOURCE_DIR}/../../../../../core/include
    ${CMAKE_SOURCE_DIR}/../../../../../core/third_party/simple-2d-constraint-solver/include
)

target_link_libraries(engine_sim_core PRIVATE
    simple-2d-constraint-solver
)

set_target_properties(engine_sim_core PROPERTIES
    OUTPUT_NAME "engine_sim_core"
    POSITION_INDEPENDENT_CODE ON
)

# =============================================================================
# JNI bridge shared library
# =============================================================================
add_library(engine_sim_jni SHARED
    ${CMAKE_CURRENT_SOURCE_DIR}/JniBridge.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/aaudio_engine.cpp
)

find_library(AAUDIO_LIB aaudio)

target_include_directories(engine_sim_jni PRIVATE
    ${CMAKE_SOURCE_DIR}/../../../../../core/include
)

target_link_libraries(engine_sim_jni
    engine_sim_core
    log
    android
    ${AAUDIO_LIB}
)

set_target_properties(engine_sim_jni PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    BUILD_RPATH "$ORIGIN"
    ANDROID_PLATFORM android-26
)