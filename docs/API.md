# Engine-Sim Android API 문서

## 개요

엔진 시뮬레이션 라이브러리의 공용 API를 문서화합니다.

---

## Kotlin API

### `EngineSimLibrary` (com.enginesim.app.EngineSimLibrary)

메인 엔트리포인트. JNI를 통해 네이티브 C++ 코어와 통신합니다.

#### 생성 및 해제

```kotlin
class EngineSimLibrary {
    companion object {
        init {
            System.loadLibrary("engine_sim_jni")
        }
    }

    fun create(): Boolean      // 엔진 인스턴스 생성
    fun destroy()             // 엔진 인스턴스销毁 및 리소스 해제
    fun shutdown()            // 오디오만.shutdown (destroy 호출 권장)
}
```

#### 오디오 초기화

```kotlin
fun initializeAudio(
    sampleRate: Int = 44100,       // 샘플 레이트 (Hz)
    channelCount: Int = 1,         // 채널 수 (현재 1=모노만 지원)
    bufferSizeFrames: Int = 512    // 버퍼 크기 (프레임)
): Boolean
```

#### 엔진 제어

```kotlin
fun setThrottle(throttle: Float)   // 스로틀 위치 설정 (0.0 ~ 1.0)
fun setVolume(volume: Float)       // 볼륨 설정 (0.0 ~ 1.0)
```

#### 엔진 상태 조회

```kotlin
fun getRpm(): Float                // 현재 RPM
fun getTorque(): Float             // 현재 토크 (N*m)
fun getPower(): Float              // 현재 출력 (W)
fun getSpeed(): Float              // 차량 속도 (m/s)
fun isSpinning(): Boolean          // 엔진 회전 중 여부
```

#### 오디오

```kotlin
fun readAudio(buffer: ShortArray, samples: Int): Int
// 반환: 실제 읽은 샘플 수
// buffer: PCM 16-bit 샘플 버퍼
// samples: 읽을 샘플 수
```

---

### `AudioEngine` (com.enginesim.app.AudioEngine)

AudioTrack 기반 오디오 플레이어. Android 5.0 (API 21) 이상 지원.

#### 생성

```kotlin
class AudioEngine(
    private val sampleRate: Int = 44100,
    private val channelCount: Int = 1,
    private val bufferSizeFrames: Int = 512
)
```

#### 메서드

```kotlin
fun initialize(): Boolean
fun writeAudio(buffer: ShortArray, samples: Int): Int
fun setVolume(volume: Float)
fun shutdown()
```

---

## JNI 함수 (C++)

네이티브 메서드 시그니처 (Java_com_enginesim_app_EngineSimLibrary_native*).

### 라이프사이클

| JNI 함수 | 설명 |
|----------|------|
| `jlong nativeCreate()` | 엔진 시뮬레이터 인스턴스 생성. `EngineSimHandle*` 포인터 반환 |
| `void nativeDestroy(jlong handle)` | 인스턴스销毁 및 메모리 해제 |

### 오디오

| JNI 함수 | 설명 |
|----------|------|
| `jboolean nativeInitializeAudio(jlong handle, jint sampleRate, jint channelCount, jint bufferSizeFrames)` | AAudio 오류 시 fallback 사용 |
| `void nativeSetVolume(jlong handle, jfloat volume)` | 볼륨 설정 (0.0 ~ 1.0) |
| `jint nativeReadAudio(jlong handle, jshortArray buffer, jint samples)` | PCM 샘플 읽기. 직접 버퍼 접근 (`GetShortArrayElements`) |

### 엔진 제어

| JNI 함수 | 설명 |
|----------|------|
| `void nativeSetThrottle(jlong handle, jfloat throttle)` | 스로틀 위치 설정 |
| `jfloat nativeGetRpm(jlong handle)` | RPM 조회 |
| `jfloat nativeGetTorque(jlong handle)` | 토크 조회 |
| `jfloat nativeGetPower(jlong handle)` | 출력 조회 |

### 시뮬레이션

| JNI 함수 | 설명 |
|----------|------|
| `void nativeStart(jlong handle)` | 시뮬레이션 시작 |
| `void nativeStop(jlong handle)` | 시뮬레이션 정지 |
| `jboolean nativeSimulateStep(jlong handle)` | 단일 스텝 실행 |

---

## 데이터 구조

### `EngineSimHandle` (C++)

```cpp
struct EngineSimHandle {
    std::unique_ptr<engine_sim::Simulator> simulator;
    std::unique_ptr<engine_sim::Synthesizer> synthesizer;
    std::unique_ptr<engine_sim::AndroidAudioOutput> audioOutput;
    float currentRpm = 0.0f;
    float volume = 1.0f;
};
```

### 오디오 포맷

| 항목 | 값 |
|------|-----|
| 인코딩 | PCM 16-bit signed integer |
| 샘플 레이트 | 44100 Hz |
| 채널 | 1 (모노) |
| 엔디안 | 리틀엔디안 |

---

## 사용 예제

### 기본 사용 (Kotlin)

```kotlin
class MainActivity : AppCompatActivity() {

    private lateinit var engine: EngineSimLibrary

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // 엔진 생성
        engine = EngineSimLibrary()
        if (!engine.create()) {
            Log.e("EngineSim", "Failed to create engine")
            return
        }

        // 오디오 초기화
        if (!engine.initializeAudio()) {
            Log.e("EngineSim", "Failed to initialize audio")
        }

        // 볼륨 설정
        engine.setVolume(0.8f)

        // 오디오 쓰레드 시작
        startAudioLoop()
    }

    private fun startAudioLoop() {
        val buffer = ShortArray(512)
        CoroutineScope(Dispatchers.Default).launch {
            while (isActive) {
                val samplesRead = engine.readAudio(buffer, 512)
                if (samplesRead == 0) {
                    delay(1) // 빈 버퍼면 잠시 대기
                }
            }
        }
    }

    // 스로틀 조작 (예: 슬라이더)
    fun setThrottle(value: Float) {
        engine.setThrottle(value.coerceIn(0f, 1f))
    }

    override fun onDestroy() {
        super.onDestroy()
        engine.destroy() // 중요: 리소스 해제
    }
}
```

### 오디오 콜백 통합 (AAudio)

```kotlin
// AudioEngine.kt의 오디오 루프
private suspend fun writeAudioLoop() {
    while (isActive) {
        val samplesRead: Int
        synchronized(bufferLock) {
            val readPos = (bufferWritePos - ringBufferSize + bufferSize)
                .mod(ringBufferSize)
            samplesRead = minOf(bufferSize, ringBufferSize - bufferWritePos + readPos)
            if (samplesRead > 0) {
                System.arraycopy(ringBuffer, readPos, tempBuffer, 0, samplesRead)
                bufferWritePos = (bufferWritePos + samplesRead) % ringBufferSize
            }
        }

        if (samplesRead > 0) {
            audioTrack?.write(tempBuffer, 0, samplesRead)
        } else {
            delay(1)
        }
    }
}
```

---

## 제한 사항

1. **모노 오디오만 지원** — 스테레오는 현재 미구현
2. **동기식 오디오 읽기** — `readAudio()`는 블록킹 호출
3. **NDK 버전** — NDK r21 이상 필수 (AAudio API 26+)