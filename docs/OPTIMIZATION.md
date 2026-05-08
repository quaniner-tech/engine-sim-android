# Engine-Sim Android 최적화 & 성능 튜닝 가이드

**작성일**: 2026-05-07  
**대상**: engine-sim-android 안드로이드 라이브러리  
**목표**: 10kHz 시뮬레이션 + 저지연 오디오를 Android에서 최적화

---

## 1. 시뮬레이션 스레드 최적화

### 1.1 스레드 구조 분석

현재 `Simulator`는 `simulateStep()`을 외부(앱 또는 JNI 콜)에서 주기적으로 호출하는 구조:

```
앱 스레드 (Vsync 또는 Timer)
  └─ Simulator::startFrame(dt)
      └─ for (i < m_steps): simulateStep()
          └─ m_system->process(timestep, 1)
```

**문제점**: 앱 스레드에서 시뮬레이션을 실행하면 GC, UI 렌더링에 의해 간섭을 받을 수 있음.

### 1.2 전용 시뮬레이션 스레드 생성

`PistonEngineSimulator` 또는 래퍼 클래스에서 전용 스레드를 생성:

```cpp
// simulation_thread.h
#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <sched.h>
#include <pthread.h>

#include "piston_engine_simulator.h"

namespace engine_sim {

class SimulationThread {
public:
    SimulationThread();
    ~SimulationThread();

    // Start dedicated simulation thread at 10kHz
    bool start(PistonEngineSimulator* simulator, int frequency = 10000);
    void stop();

    bool isRunning() const { return m_running.load(); }

    // Priority: SCHED_FIFO (requires root) or high nice
    void setPriority(int priority);   // 1-99 for SCHED_FIFO
    void setNice(int niceValue);      // -20 to 19

    // Pin to specific CPU cores (avoid big.LITTLE migration)
    void setCpuAffinity(const std::vector<int>& cores);

private:
    void runLoop();

    std::thread* m_thread = nullptr;
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_pause{false};

    PistonEngineSimulator* m_simulator = nullptr;
    int m_frequency = 10000;

    std::chrono::steady_clock::time_point m_lastStep;
    double m_stepInterval;  // nanoseconds (100us for 10kHz)
};

} // namespace engine_sim
```

```cpp
// simulation_thread.cpp
#include "simulation_thread.h"

namespace engine_sim {

SimulationThread::SimulationThread() {}

SimulationThread::~SimulationThread() {
    stop();
}

bool SimulationThread::start(PistonEngineSimulator* sim, int frequency) {
    if (m_running.load()) return false;

    m_simulator = sim;
    m_frequency = frequency;
    m_stepInterval = 1'000'000'000.0 / frequency;  // nanoseconds
    m_running.store(true);

    m_thread = new std::thread(&SimulationThread::runLoop, this);
    return true;
}

void SimulationThread::stop() {
    if (!m_running.load()) return;
    m_running.store(false);
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
    }
    delete m_thread;
    m_thread = nullptr;
}

void SimulationThread::setPriority(int priority) {
    if (!m_thread) return;

    sched_param param;
    param.sched_priority = priority;
    pthread_setschedparam(m_thread->native_handle(), SCHED_FIFO, &param);
}

void SimulationThread::setCpuAffinity(const std::vector<int>& cores) {
    if (!m_thread || cores.empty()) return;

    cpu_set_t cpuSet;
    CPU_ZERO(&cpuSet);
    for (int core : cores) {
        CPU_SET(core, &cpuSet);
    }
    pthread_setaffinity_np(m_thread->native_handle(), sizeof(cpu_set_t), &cpuSet);
}

void SimulationThread::runLoop() {
    using namespace std::chrono;

    // Try SCHED_FIFO first (needs CAP_SYS_NICE or root)
    setPriority(90);  // Realtime audio priority

    // Pin to CPU core 0-1 (performance cluster on big.LITTLE)
    setCpuAffinity({0, 1});

    m_lastStep = steady_clock::now();

    while (m_running.load()) {
        if (m_pause.load()) {
            std::this_thread::yield();
            continue;
        }

        auto targetTime = m_lastStep + nanoseconds(static_cast<long>(m_stepInterval));
        std::this_thread::sleep_until(targetTime);

        // Run one simulation step (10kHz = 100us per step)
        if (m_simulator) {
            m_simulator->simulateStep();
        }

        m_lastStep = steady_clock::now();
    }
}

} // namespace engine_sim
```

### 1.3 CPU 친화도 (Affinity) 설정 가이드

Android 기기의 big.LITTLE 구조에서 시뮬레이션 스레드를 **performance cluster(Cortex-A7x 계열)** 에 고정:

```kotlin
// AndroidCpuManager.kt
object AndroidCpuManager {
    // Get list of CPU cores
    fun getPerformanceCores(): List<Int> {
        // /sys/devices/system/cpu/cpu0/cpufreq/scaling_available_frequencies
        // 성능 클러스터 코어만 반환 (대략 0-3)
        return listOf(0, 1, 2, 3).filter { isCoreOnline(it) }
    }

    fun isCoreOnline(core: Int): Boolean {
        return File("/sys/devices/system/cpu/cpu$core/online").readText().trim() == "1"
    }

    fun setThreadAffinity(thread: Long, cores: List<Int>) {
        val cpuSet = android.os.CpuBitmap.toCpuBitmap(cores)
        android.os.Process.setThreadCpuAffinity(thread, cpuSet)
    }
}
```

### 1.4 스레드 우선순위 (Android Manifest)

```xml
<!-- AndroidManifest.xml -->
<uses-permission android:name="android.permission.HIGH_SAMPLING_RATE_SENSORS" />
<uses-permission android:name="android.permission.REAL_GET_TASKS" /> <!-- API 31+ -->
```

---

## 2. 오디오 레이턴시 최소화

### 2.1 AAudio MMAP support 확인

Android 8.0+ (API 26+)에서 AAudio MMAP을 지원하는지 확인:

```cpp
// aaudio_mmap_check.h
#pragma once

#include <aaudio/AAudio.h>
#include <android/log.h>

class AAudioMmapChecker {
public:
    // Check if MMAP (low-latency) is supported
    static bool isMmapSupported() {
        // AAudio v26+ always uses MMAP when available
        // No explicit MMAP check needed - AAudio uses it automatically
        // if the device supports it.
        //
        // To check if a specific stream uses MMAP:
        // - Create stream with PERFORMANCE_MODE_LOW_LATENCY
        // - Check AAUDIO_PERFORMANCE_MODE_LOW_LATENCY after start
        return true;
    }

    // Get optimal buffer size for minimum latency
    static int32_t getOptimalBufferSize(AAudioStream* stream) {
        // AAudio chooses optimal size based on device capabilities
        // Query current framesPerBurst:
        int32_t burst = AAudioStream_getFramesPerBurst(stream);

        // Double buffer for stability (2 * burst = ~11.6ms at 44.1kHz)
        return burst * 2;
    }

    // Check if using MMAP (hidden method via dladdr)
    static bool isUsingMmap(AAudioStream* stream) {
        // Not directly queryable - heuristic:
        // Low-latency mode + small buffer = MMAP
        return AAudioStream_getPerformanceMode(stream) ==
               AAUDIO_PERFORMANCE_MODE_LOW_LATENCY;
    }
};
```

### 2.2 버퍼 크기 자동 조정 (디바이스별 최적화)

```cpp
// buffer_size_optimizer.h
#pragma once

#include <aaudio/AAudio.h>
#include <atomic>

class BufferSizeOptimizer {
public:
    struct DeviceProfile {
        int32_t defaultBufferFrames;
        int32_t minBufferFrames;
        int32_t maxBufferFrames;
        int32_t burstFrames;  // From AAudio
        float latencyMs;
    };

    // Known device profiles (add more as tested)
    static DeviceProfile getDeviceProfile(const char* manufacturer,
                                          const char* model) {
        // Pixel 6/7 (Snapdragon 888+)
        if (strcmp(manufacturer, "Google") == 0) {
            return {512, 128, 4096, 192, 5.8f};
        }
        // Samsung Galaxy S20+ (Exynos 990)
        if (strcmp(manufacturer, "Samsung") == 0) {
            return {512, 256, 4096, 256, 6.0f};
        }
        // Generic fallback
        return {1024, 256, 8192, 512, 12.0f};
    }

    // Adaptive buffer sizing during runtime
    static int32_t adaptBufferSize(AAudioStream* stream,
                                    int32_t currentUnderruns) {
        int32_t burst = AAudioStream_getFramesPerBurst(stream);
        int32_t current = AAudioStream_getBufferSize(stream);

        if (currentUnderruns > 10) {
            // Too many underruns - increase buffer
            return std::min(current * 2, burst * 8);
        } else if (currentUnderruns == 0 && current > burst * 2) {
            // No underruns, try smaller buffer for lower latency
            return std::max(current / 2, burst * 2);
        }
        return current;
    }
};
```

### 2.3 BURST 기반 버퍼 할당

```cpp
// AAudioEngine.cpp (개선된 버전)
#include "aaudio_engine.h"
#include "buffer_size_optimizer.h"
#include "simulation_thread.h"

bool AAudioEngine::initialize(int sampleRate, int channelCount,
                               int bufferSizeFrames) {
    // ... existing code ...

    // Query optimal burst size
    AAudioStream_builder_setFramesPerBurst(stream, 0);  // 0 = auto

    // Create stream
    aaudio_result_t result = AAudioStream_open(builder, &stream);
    if (result != AAUDIO_OK) {
        return false;
    }

    // Get actual burst size
    int32_t burstFrames = AAudioStream_getFramesPerBurst(stream);

    // Set buffer to 2x burst (safe default)
    int32_t bufferFrames = bufferSizeFrames > 0
        ? bufferSizeFrames
        : burstFrames * 2;

    AAudioStream_setBufferSize(stream, bufferFrames);

    __android_log_print(ANDROID_LOG_INFO, "AAudioEngine",
        "Buffer: %d frames, Burst: %d frames, ~%.1f ms latency",
        bufferFrames, burstFrames,
        (float)bufferFrames / sampleRate * 1000.f);

    return true;
}
```

---

## 3. 메모리 최적화

### 3.1 PCM 버퍼 풀 사전 할당 (GC 방지)

```cpp
// pcm_buffer_pool.h
#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <cstdint>

class PcmBufferPool {
public:
    struct Buffer {
        int16_t* data;
        int capacity;
        std::atomic<bool> inUse{false};
    };

    PcmBufferPool() = default;

    ~PcmBufferPool() {
        for (auto& buf : m_buffers) {
            delete[] buf.data;
        }
    }

    void initialize(int bufferCount, int samplesPerBuffer) {
        m_buffers.reserve(bufferCount);
        for (int i = 0; i < bufferCount; ++i) {
            Buffer buf;
            buf.capacity = samplesPerBuffer;
            buf.data = new int16_t[samplesPerBuffer];
            buf.inUse.store(false);
            m_buffers.push_back(std::move(buf));
        }
    }

    Buffer* acquire() {
        for (auto& buf : m_buffers) {
            bool expected = false;
            if (buf.inUse.compare_exchange_strong(expected, true)) {
                return &buf;
            }
        }
        return nullptr;  // All buffers in use
    }

    void release(Buffer* buf) {
        buf->inUse.store(false);
    }

private:
    std::vector<Buffer> m_buffers;
};
```

### 3.2 JNI 글로벌 참조 최소화

```cpp
// JniBridge.cpp - 개선된 버전
extern "C" {

// Use local reference batch processing
JNIEXPORT jlong JNICALL
Java_com_enginesim_app_EngineSimLibrary_nativeCreate(JNIEnv* env,
                                                     jobject thiz) {
    // Cache classes and method IDs at load time, not per-call
    // (Use JNI_OnLoad to cache)
    LOGI("Creating EngineSim handle");

    // No NewGlobalRef needed for simple types
    // For objects: use WeakGlobalRef if persistent but not permanent
    EngineSimHandle* handle = new EngineSimHandle();

    return reinterpret_cast<jlong>(handle);
}

JNIEXPORT void JNICALL
Java_com_enginesim_app_EngineSimLibrary_nativeDestroy(JNIEnv* env,
                                                     jobject thiz,
                                                     jlong handle) {
    if (handle != 0) {
        EngineSimHandle* h = reinterpret_cast<EngineSimHandle*>(handle);
        // Release any lingering global refs here
        delete h;
    }
}

} // extern "C"

// JNI_OnLoad - cache method IDs (called once at library load)
jclass g_engineSimLibraryClass = nullptr;
jmethodID g_nativeCreate_method = 0;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_VERSION_1_6;
    }

    // Cache class reference (global)
    jclass clazz = env->FindClass("com/enginesim/app/EngineSimLibrary");
    if (clazz != nullptr) {
        g_engineSimLibraryClass = static_cast<jclass>(
            env->NewGlobalRef(clazz));
        env->DeleteLocalRef(clazz);

        // Cache method IDs (don't need global ref for method IDs)
        g_nativeCreate_method = env->GetMethodID(
            g_engineSimLibraryClass, "nativeCreate", "()J");
    }

    return JNI_VERSION_1_6;
}
```

### 3.3 네이티브 메모리 누수 방지 체크리스트

| 체크포인트 | 방법 |
|-----------|------|
| `delete` 누락 | `std::unique_ptr` 사용으로 자동 삭제 보장 |
| `new[]` 배열 | `std::vector` 또는 `std::unique_ptr<T[]>` 사용 |
| JNI Global Ref | `NewGlobalRef` → `DeleteGlobalRef` 페어 관리 |
| `RingBuffer` 소멸 | `destroy()` 명시적 호출 확인 |
| 스레드 정리 | `join()` 또는 `detach()` 확인 |

---

## 4. 네이티브 코드 최적화

### 4.1 컴파일 플래그 (-O3 + LTO)

**Android CMakeLists.txt (android/app/src/main/cpp/CMakeLists.txt)**:

```cmake
# =============================================================================
# Compiler Optimization Flags
# =============================================================================
# ARM64: -O3 + LTO for maximum performance
# NOTE: LTO can increase compile time significantly
if (ANDROID_ABI STREQUAL "arm64-v8a")
    set(OPT_FLAGS
        -O3
        -ffast-math
        -funsafe-math-optimizations
        -flto
        -finline-functions
        -fno-stack-protector
        -march=armv8-a+crc+crypto+simd  # NEON + CRC + Crypto
    )
elseif (ANDROID_ABI STREQUAL "armeabi-v7a")
    set(OPT_FLAGS
        -O3
        -ffast-math
        -funsafe-math-optimizations
        -flto
        -finline-functions
        -fno-stack-protector
        -march=armv7-a-neon
    )
endif()

add_compile_options(${OPT_FLAGS})

# For core library (used by both JNI and potential static link)
set_target_properties(engine_sim_core PROPERTIES
    COMPILE_FLAGS "-O3 -ffast-math -flto"
)

# =============================================================================
# Link-Time Optimization (LTO)
# =============================================================================
if (ANDROID_ABI STREQUAL "arm64-v8a")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -Wl,--lto-O3")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -flto -Wl,--lto-O3")
endif()
```

### 4.2 NEON SIMD 활용

ARM NEON intrinsics를 사용해 핫 패스 최적화:

```cpp
// neon_optimization.h
#pragma once

#include <arm_neon.h>

// Check NEON availability
#if defined(__ARM_NEON) || defined(__aarch64__)

// Volume application with NEON (16 samples at once)
inline void applyVolumeNeon(int16_t* dest, const int16_t* src,
                             float volume, int count) {
    // Count must be multiple of 16
    int16_t vol_s16 = static_cast<int16_t>(volume * 32767.0f);

    // Load volume as float32x4 for multiplication
    float32x4_t vol_vec = vdupq_n_f32(volume);

    for (int i = 0; i < count; i += 16) {
        // Load 16 int16 samples
        int16x8_t s0 = vld1q_s16(src + i);
        int16x8_t s1 = vld1q_s16(src + i + 8);

        // Convert to float32
        float32x4_t f0 = vcvtq_f32_s32(vmovl_s16(vget_low_s16(s0)));
        float32x4_t f1 = vcvtq_f32_s32(vmovl_s16(vget_high_s16(s0)));
        float32x4_t f2 = vcvtq_f32_s32(vmovl_s16(vget_low_s16(s1)));
        float32x4_t f3 = vcvtq_f32_s32(vmovl_s16(vget_high_s16(s1)));

        // Multiply by volume
        f0 = vmulq_f32(f0, vol_vec);
        f1 = vmulq_f32(f1, vol_vec);
        f2 = vmulq_f32(f2, vol_vec);
        f3 = vmulq_f32(f3, vol_vec);

        // Clamp + convert back
        int32x4_t r0 = vcvtaq_s32_f32(f0);
        int32x4_t r1 = vcvtaq_s32_f32(f1);
        int32x4_t r2 = vcvtaq_s32_f32(f2);
        int32x4_t r3 = vcvtaq_s32_f32(f3);

        // Narrow to int16
        int16x4_t o0 = vqmovn_s32(r0);
        int16x4_t o1 = vqmovn_s32(r1);
        int16x4_t o2 = vqmovn_s32(r2);
        int16x4_t o3 = vqmovn_s32(r3);

        // Store
        int16x8_t out0 = vcombine_s16(o0, o1);
        int16x8_t out1 = vcombine_s16(o2, o3);
        vst1q_s16(dest + i, out0);
        vst1q_s16(dest + i + 8, out1);
    }
}

#else
// Scalar fallback
inline void applyVolumeNeon(int16_t* dest, const int16_t* src,
                             float volume, int count) {
    int16_t vol_s16 = static_cast<int16_t>(volume * 32767.0f);
    for (int i = 0; i < count; ++i) {
        dest[i] = static_cast<int16_t>(
            (static_cast<int32_t>(src[i]) * vol_s16) >> 15);
    }
}
#endif
```

### 4.3 핫 패스 인라인 힌트

```cpp
// Hot path inline directives
class Synthesizer {
public:
    // CRITICAL: Called 10,000 times per second - MUST inline
    __attribute__((always_inline))
    inline int16_t renderAudio(int inputOffset) {
        // Audio rendering hot path
        // ...
    }

    // Called from audio thread - optimize heavily
    __attribute__((always_inline))
    inline void writeInput(const double* data) {
        // Input writing hot path
        // ...
    }
};

// Ring buffer hot paths
template <typename T_Data>
class RingBuffer {
public:
    __attribute__((always_inline))
    inline void write(T_Data data) {
        m_buffer[m_writeIndex] = data;
        if (++m_writeIndex >= m_capacity) {
            m_writeIndex = 0;
        }
    }

    __attribute__((always_inline))
    inline T_Data read(size_t index) const {
        return (m_start + index) >= m_capacity
            ? m_buffer[m_start + index - m_capacity]
            : m_buffer[m_start + index];
    }
};
```

---

## 5. 프로파일링 가이드

### 5.1 simpleperf (네이티브 프로파일링)

```bash
# Device setup (Android 10+)
# Connect device, enable developer mode, USB debugging

# Install simpleperf (if not already present)
adb root
adb install simpleperf.apk  # From NDK

# Record CPU profile (60 seconds, app only)
adb shell simpleperf record -p $(adb shell pidof com.enginesim.app) \
    -f 1000 \           # Sample frequency 1000Hz
    --call-graph fp \   # Frame pointer unwinding
    -o /data/local/tmp/perf.data \
    --duration 60

# Download and analyze on host
adb pull /data/local/tmp/perf.data ./engine_sim_profile.perf

# Report generation
adb shell simpleperf report \
    -i /data/local/tmp/perf.data \
    --call-graph > perf_report.txt

# Show hot functions (top 50)
simpleperf report -i engine_sim_profile.perf --stdio | head -50

# Show which functions call "simulateStep"
simpleperf report -i engine_sim_profile.perf \
    -g caller \
    --stdio | grep -A5 "simulateStep"
```

### 5.2 Android systrace (오디오 레이턴시 측정)

```bash
# Record systrace (30 seconds during audio playback)
python3 $ANDROID_SDK/platform-tools/systrace/systrace.py \
    --time=30 \
    -o engine_sim_trace.html \
    gfx input view webview wm am audio video \
    sched freq idle disk load

# Analyze trace
# Open engine_sim_trace.html in Chrome (chrome://tracing)
#
# Key markers to look for:
# - aaudio callback latency: "AAudioThread" 
# - buffer underruns: Look for "underrun" label
# - scheduling delay: Audio thread vs expected callback time
```

### 5.3 Custom Performance Markers (C++)

```cpp
// Custom trace markers (visible in systrace)
#include <android/api-level.h>

#if __ANDROID_API__ >= 29
#include <sysTrace/Trace.h>

#define ATRACE_NAME(name) android::Trace::traceBegin(ATRACE_TAG, name)
#define ATRACE_END() android::Trace::traceEnd(ATRACE_TAG)
#define ATRACE_INT(name, value) android::Trace::traceCounter(ATRACE_TAG, name, value)
#else
#define ATRACE_NAME(name)
#define ATRACE_END()
#define ATRACE_INT(name, value)
#endif

// Usage in simulator.cpp
void Simulator::simulateStep() {
    ATRACE_NAME("simulateStep");
    // ... simulation code ...
    ATRACE_INT("rpm", static_cast<int>(m_engine->getRpm()));
    ATRACE_END();
}
```

### 5.4 Jank Detection (UI + Audio sync)

```kotlin
// MainActivity.kt
class MainActivity : ComponentActivity() {
    private var lastFrameTime = System.nanoTime()
    private val frameTimes = mutableListOf<Long>()

    override fun onFrame(frameTimeNanos: Long) {
        val delta = frameTimeNanos - lastFrameTime
        lastFrameTime = frameTimeNanos

        frameTimes.add(delta)
        if (frameTimes.size > 60) {
            val avg = frameTimes.average()
            val jitter = frameTimes.map { kotlin.math.abs(it - avg) }.average()

            // Log if frame time > 20ms (below 50fps)
            if (avg > 20_000_000) {
                Log.w("PERF", "Low FPS: avg=${avg/1_000_000}ms, jitter=${jitter/1_000_000}ms")
            }
            frameTimes.clear()
        }
    }
}
```

---

## 6. 권장 파라미터 값

| 시나리오 | 주파수 | 버퍼 | 지연 예상 | 용도 |
|---------|--------|------|----------|------|
| 저지연 | 10kHz | 256 frames | ~6ms | 빠른 응답 |
| 밸런스 | 10kHz | 512 frames | ~12ms | 일반 게임 |
| 안정성 | 10kHz | 1024 frames | ~23ms | 블루투스 동시 |

---

## 7. 빌드 검증

```bash
# Release 빌드 (ndk-build 또는 CMake)
cd android && ./gradlew assembleRelease

# 네이티브 라이브러리 확인
$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/llvm-readelf \
    -S app/build/.cxx/release/*/libengine_sim_jni.so \
    | grep -i text

# 결과 확인 (TEXT 섹션 크기)
# Text 크기 < 2MB 목표 (NDK에서 확인)
```

---

## 8. 성능 벤치마크 체크리스트

- [ ] 10kHz simulation thread CPU 사용률 < 15%
- [ ] 오디오 레이턴시 (AAudio 콜백 → 스피커 출력) < 20ms
- [ ] 프레임 드랍 (UI) < 1%
- [ ] 버퍼 언더런 카운트 < 5회/분
- [ ] 메모리 사용량 (PSS) < 50MB
- [ ] cold start → 오디오 출력까지 < 2초
