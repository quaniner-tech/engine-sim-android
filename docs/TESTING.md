# Testing Guide — Engine Sim Android

## Overview

This document describes the testing infrastructure for the engine-sim-android project.

## Test Structure

```
android/app/src/
├── test/
│   ├── cpp/                        # C++ Google Test unit tests
│   │   ├── CMakeLists.txt
│   │   ├── test_engine.cpp          # Engine & PistonEngineSimulator tests
│   │   ├── test_synthesizer.cpp    # Synthesizer & audio pipeline tests
│   │   └── test_piston.cpp         # Piston component tests
│   └── java/com/enginesim/         # Kotlin JUnit tests
│       └── EngineSimLibraryTest.kt  # JNI wrapper unit tests
└── androidTest/
    └── java/com/enginesim/         # Android instrumented tests
        └── EngineSimInstrumentedTest.kt  # Integration with UI/audio
```

## Running Tests

### C++ Unit Tests

```bash
# From the project root
mkdir -p android/app/src/test/cpp/build
cd android/app/src/test/cpp/build

# Configure with Android NDK toolchain (or host toolchain for native testing)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# On Linux/macOS (native testing without Android NDK):
# cmake .. -DCMAKE_BUILD_TYPE=Debug -DANDROID_ABI=x86_64

# Build
cmake --build . -- -j$(nproc)

# Run tests
ctest --output-on-failure
./test_engine
./test_synthesizer
./test_piston
```

### Kotlin Unit Tests

```bash
cd android
./gradlew testDebugUnitTest
```

### Android Instrumented Tests (requires device/emulator)

```bash
cd android
./gradlew connectedAndroidTest
```

Or with a specific device:
```bash
./gradlew testDebugAndroidTest -Pandroid.testInstrumentationRunnerArguments.device=emulator-5554
```

## Test Categories

### 1. C++ Unit Tests (`test_engine.cpp`, `test_synthesizer.cpp`, `test_piston.cpp`)

**Engine tests:**
- `EngineTest.Construction` — Engine object construction
- `EngineTest.InitializeInline4` — Inline-4 engine initialization
- `EngineTest.InitializeV8` — V8 engine initialization
- `EngineTest.ThrottleControl` — Throttle value setting and retrieval
- `EngineTest.SpeedControl` — Speed control (dynamometer) mode
- `EngineTest.RedlineAndDynoRanges` — Parameter validation
- `EngineTest.EngineGetters` — Crankshaft, cylinder bank, fuel system access
- `EngineTest.PistonAccess` — Piston array access
- `EngineTest.SimulationParameters` — Simulation frequency, noise settings
- `EngineTest.TorqueAndPowerGetters` — Manifold pressure, AFR, exhaust O2
- `EngineTest.SpinningState` — Engine spinning state
- `EngineTest.FuelConsumption` — Fuel tracking
- `EngineTest.EngineDisplacement` — Displacement calculation
- `PistonEngineSimulatorTest.CreateAndDestroy` — Simulator lifecycle
- `PistonEngineSimulatorTest.SimulationFrequency` — Frequency setting
- `PistonEngineSimulatorTest.Timestep` — Timestep calculation

**Synthesizer tests:**
- `SynthesizerTest.Construction` — Synthesizer construction
- `SynthesizerTest.Initialize` — Single-channel initialization
- `SynthesizerTest.InitializeDualChannel` — Dual-channel initialization
- `SynthesizerTest.Destroy` — Cleanup
- `SynthesizerTest.ReadAudioOutputEmpty` — Reading from empty buffer
- `SynthesizerTest.ReadAudioOutputPartial` — Partial reads
- `SynthesizerTest.ReadAudioOutputLargerThanBuffer` — Oversized read requests
- `SynthesizerTest.SampleRates` — Sample rate get/set
- `SynthesizerTest.AudioParameters` — Audio parameter initialization
- `SynthesizerTest.SetAudioParameters` — Parameter mutation
- `SynthesizerTest.Latency` — Latency measurement
- `SynthesizerTest.ImpulseResponseInitialization` — IR loading
- `SynthesizerTest.InputWrite` — Input data writing
- `SynthesizerTest.InputDelta` — Delta calculation
- `SynthesizerTest.InputDistance` — Distance calculation

**Piston tests:**
- `PistonTest.Construction` — Piston construction
- `PistonTest.PistonGetters` — Default values before init
- `PistonTest.PistonMassAndBlowby` — Mass and blowby coefficient
- `PistonTest.RelativePosition` — Position calculation
- `PistonTest.InitializeWithParams` — Parameterized initialization
- `PistonTest.CalculateCylinderWallForce` — Force calculation
- `PistonTest.WristPinLocation` — Wrist pin position
- `PistonTest.MultiplePistons` — Multiple piston management
- `PistonTest.Destroy` — Cleanup

### 2. Kotlin Unit Tests (`EngineSimLibraryTest.kt`)

Tests the JNI wrapper without requiring Android device:
- Library creation/destruction lifecycle
- RPM, torque, power, speed readings
- Throttle and speed control setting
- Engine preset loading (Inline4, V6, V8, V12)
- Configuration loading
- Audio buffer reading
- Volume control
- Sample rate queries
- Simulation stepping

### 3. Android Instrumented Tests (`EngineSimInstrumentedTest.kt`)

Requires Android device/emulator, tests actual JNI integration:
- Native library loading
- Valid JNI handle creation
- Audio rendering thread startup
- Continuous audio output
- Throttle changes affecting audio
- Engine preset switching
- Memory stability (repeated create/destroy)
- JNI stability under load
- Sample rate correctness
- Simulation stepping
- Volume control
- Start/stop

## CI Configuration

See [`.github/workflows/android.yml`](../../.github/workflows/android.yml).

### C++ Tests CI
- Runs on Ubuntu with NDK toolchain
- Caches NDK to speed up builds
- Builds test binaries and runs with CTest
- Falls back to direct binary execution if CTest fails

### Android Tests CI
- Runs on macOS (or Ubuntu with emulator)
- Caches Gradle and Android SDK
- Builds debug APK
- Runs unit tests via `./gradlew testDebugUnitTest`
- Uploads test reports as artifacts

## Adding New Tests

### C++ Tests

Add new test files in `android/app/src/test/cpp/` and update `CMakeLists.txt`:

```cpp
// In test_new_component.cpp
#include <gtest/gtest.h>
#include <engine_sim/new_component.h>

TEST(NewComponentTest, SomeBehavior) {
    NewComponent c;
    // ...
}
```

Then add to `CMakeLists.txt`:
```cmake
add_executable(test_new test_new_component.cpp)
target_link_libraries(test_new engine_sim_core_test gtest gtest_main)
gtest_add_tests(TARGETS test_new)
```

### Kotlin Tests

Add to existing test class or create new file in `android/app/src/test/java/com/enginesim/`:

```kotlin
@Test
fun `new behavior test`() {
    // ...
}
```

### Instrumented Tests

Add to `android/app/src/androidTest/java/com/enginesim/EngineSimInstrumentedTest.kt`
or create new test class. Instrumented tests require `@RunWith(AndroidJUnit4::class)`.

## Known Limitations

1. **C++ tests on non-Android**: The test CMake uses the Android NDK toolchain by default. For native testing on Linux/macOS, pass `-DCMAKE_TOOLCHAIN_FILE` for the appropriate NDK or use a native toolchain.

2. **Instrumented tests**: Require either an Android emulator or a physical device with USB debugging enabled. CI may need additional setup for emulator start/stop.

3. **Audio output tests**: These are best-effort; the exact audio characteristics depend on synthesis parameters and timing.

4. **SCS dependency**: C++ tests link against `simple-2d-constraint-solver` (SCS) which is included as a subdirectory.

## Continuous Integration Status

| Job | Platform | Status |
|-----|----------|--------|
| cpp-tests | Ubuntu + NDK | ✅ Configured |
| android-unit-tests | macOS/ubuntu + Gradle | ✅ Configured |
| android-instrumented-tests | Requires emulator | ⚠️ Requires CI emulator setup |