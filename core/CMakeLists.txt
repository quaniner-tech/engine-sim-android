cmake_minimum_required(VERSION 3.10)
project(engine_sim_core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ========================================================
# Compiler Flags
# ========================================================
# RTTI and exceptions disabled to reduce binary size
add_compile_options(
    -fno-rtti
    -fno-exceptions
    -fvisibility=hidden
)

# ========================================================
# Core Simulation Sources
# ========================================================
set(ENGINE_SIM_CORE_SOURCES
    # Physics/Simulation core
    src/engine_sim/simulator.cpp
    src/engine_sim/piston_engine_simulator.cpp
    src/engine_sim/engine.cpp
    src/engine_sim/piston.cpp
    src/engine_sim/crankshaft.cpp
    src/engine_sim/cylinder_bank.cpp
    src/engine_sim/connecting_rod.cpp
    src/engine_sim/combustion_chamber.cpp
    src/engine_sim/cylinder_head.cpp
    src/engine_sim/exhaust_system.cpp
    src/engine_sim/intake.cpp
    src/engine_sim/fuel.cpp
    src/engine_sim/ignition_module.cpp
    src/engine_sim/valvetrain.cpp
    src/engine_sim/standard_valvetrain.cpp
    src/engine_sim/vtec_valvetrain.cpp
    src/engine_sim/camshaft.cpp
    src/engine_sim/throttle.cpp
    src/engine_sim/transmission.cpp
    src/engine_sim/vehicle.cpp
    src/engine_sim/vehicle_drag_constraint.cpp
    src/engine_sim/starter_motor.cpp
    src/engine_sim/dynamometer.cpp
    src/engine_sim/governor.cpp
    src/engine_sim/gas_system.cpp
    src/engine_sim/direct_throttle_linkage.cpp
    src/engine_sim/part.cpp
    
    # Audio synthesis
    src/engine_sim/synthesizer.cpp
    src/engine_sim/impulse_response.cpp
    src/engine_sim/filter.cpp
    src/engine_sim/low_pass_filter.cpp
    src/engine_sim/convolution_filter.cpp
    src/engine_sim/delay_filter.cpp
    src/engine_sim/feedback_comb_filter.cpp
    src/engine_sim/derivative_filter.cpp
    src/engine_sim/gaussian_filter.cpp
    src/engine_sim/jitter_filter.cpp
    src/engine_sim/leveling_filter.cpp
    src/engine_sim/audio_buffer.cpp
    
    # Utility
    src/engine_sim/utilities.cpp
    src/engine_sim/function.cpp
)

# ========================================================
# Header Files (for IDE support)
# ========================================================
set(ENGINE_SIM_CORE_HEADERS
    include/engine_sim/simulator.h
    include/engine_sim/engine.h
    include/engine_sim/piston.h
    include/engine_sim/crankshaft.h
    include/engine_sim/cylinder_bank.h
    include/engine_sim/connecting_rod.h
    include/engine_sim/combustion_chamber.h
    include/engine_sim/cylinder_head.h
    include/engine_sim/exhaust_system.h
    include/engine_sim/intake.h
    include/engine_sim/fuel.h
    include/engine_sim/ignition_module.h
    include/engine_sim/valvetrain.h
    include/engine_sim/standard_valvetrain.h
    include/engine_sim/vtec_valvetrain.h
    include/engine_sim/camshaft.h
    include/engine_sim/throttle.h
    include/engine_sim/transmission.h
    include/engine_sim/vehicle.h
    include/engine_sim/vehicle_drag_constraint.h
    include/engine_sim/starter_motor.h
    include/engine_sim/dynamometer.h
    include/engine_sim/governor.h
    include/engine_sim/gas_system.h
    include/engine_sim/direct_throttle_linkage.h
    include/engine_sim/part.h
    include/engine_sim/audio_buffer.h
    include/engine_sim/piston_engine_simulator.h
    
    # Audio
    include/engine_sim/synthesizer.h
    include/engine_sim/impulse_response.h
    include/engine_sim/filter.h
    include/engine_sim/low_pass_filter.h
    include/engine_sim/butterworth_low_pass_filter.h
    include/engine_sim/convolution_filter.h
    include/engine_sim/delay_filter.h
    include/engine_sim/feedback_comb_filter.h
    include/engine_sim/derivative_filter.h
    include/engine_sim/gaussian_filter.h
    include/engine_sim/jitter_filter.h
    include/engine_sim/leveling_filter.h
    
    # Utilities
    include/engine_sim/utilities.h
    include/engine_sim/function.h
    include/engine_sim/units.h
    include/engine_sim/constants.h
    include/engine_sim/scs.h
    include/engine_sim/ring_buffer.h
    
    # Interfaces
    include/engine_sim/AudioOutputInterface.h
    
    # Stub
    include/engine_sim/delta.h
)

# ========================================================
# Simple 2D Constraint Solver (SCS)
# ========================================================
add_subdirectory(third_party/simple-2d-constraint-solver)

# ========================================================
# Build engine_sim_core static library
# ========================================================
add_library(engine_sim_core STATIC
    ${ENGINE_SIM_CORE_SOURCES}
    ${ENGINE_SIM_CORE_HEADERS}
)

target_include_directories(engine_sim_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simple-2d-constraint-solver/include
)

target_link_libraries(engine_sim_core PRIVATE
    simple-2d-constraint-solver
)

target_compile_definitions(engine_sim_core PRIVATE
    -DATG_ENGINE_SIM_CORE
)

set_target_properties(engine_sim_core PROPERTIES
    OUTPUT_NAME "engine_sim_core"
    POSITION_INDEPENDENT_CODE ON
)