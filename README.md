# Engine-Sim Android Library

안드로이드용 엔진 시뮬레이션 라이브러리. 원본 [engine-sim](https://github.com/bendens1/engine-sim)의 코어를 추출하여 Android NDK 빌드 및 JNI 연동을 가능하게 만든 라이브러리입니다.

## 주요 기능

- **엔진 물리 시뮬레이션**: 피스톤, 크랭크샤프트, 배기/흡기 시스템 등
- **실시간 배기음 합성**: 오디오 신디사이저 기반排气음 생성
- **크로스 플랫폼**: C++ 코어 + Android NDK + Kotlin 래퍼
- **Low-latency 오디오**: AAudio (Android 8.0+) 지원

## 프로젝트 구조

```
engine-sim-android/
├── core/                        # C++ 시뮬레이션 코어
│   ├── include/engine_sim/      # 47개 헤더
│   ├── src/engine_sim/          # 39개 소스 파일
│   └── third_party/            # simple-2d-constraint-solver
│
├── android/                     # Android 빌드 프로젝트
│   ├── app/                     # 안드로이드 앱 모듈
│   │   ├── src/main/
│   │   │   ├── cpp/            # JNI + 네이티브 코드
│   │   │   ├── java/com/enginesim/
│   │   │   └── jni/            # JNI 헤더
│   │   └── build.gradle
│   └── lib/                     # (선택) AAR 라이브러리 모듈
│
├── docs/                        # 문서
│   ├── BUILD.md                # 빌드 가이드
│   └── API.md                  # API 문서
│
├── LICENSE
└── README.md
```

## 아키텍처

```
┌──────────────────────────────────────────────────────────────┐
│                    Kotlin Layer                              │
│  EngineSimLibrary.kt — JNI 래퍼                             │
│  AudioEngine.kt       — AudioTrack 기반 오디오 플레이어       │
├──────────────────────────────────────────────────────────────┤
│                    JNI Bridge                                │
│  JniBridge.cpp — native 메서드 구현 (handle 관리)            │
├──────────────────────────────────────────────────────────────┤
│                    C++ Core (libengine_sim_core.a)          │
│                                                              │
│  ┌────────────────┐    ┌─────────────────┐                 │
│  │  Simulator     │───▶│  Engine         │                 │
│  │  (시뮬레이션)   │    │  (피스톤/배기 등) │                 │
│  └───────┬────────┘    └─────────────────┘                 │
│          │                                                 │
│  ┌───────▼────────┐    ┌─────────────────┐                 │
│  │  Synthesizer    │───▶│  AudioBuffer    │                 │
│  │  (배기음 합성)   │    │  (PCM 출력)      │                 │
│  └───────┬────────┘    └─────────────────┘                 │
├──────────────────────────────────────────────────────────────┤
│                    Android Audio                            │
│  AndroidAudioOutput ──▶ AAudioEngine (AAudio 26+)          │
│                       └─ AudioTrack  (Legacy 21+)          │
└──────────────────────────────────────────────────────────────┘
```

## 빠른 시작

### 1. Gradle 의존성 추가 (로컬 Maven)

```kotlin
dependencies {
    implementation("com.enginesim:engine-sim-android:1.0.0")
}
```

### 2. Kotlin에서 사용

```kotlin
val engine = EngineSimLibrary()

// 엔진 생성
if (!engine.create()) {
    Log.e("EngineSim", "Failed to create engine")
    return
}

// 오디오 초기화
if (!engine.initializeAudio()) {
    Log.e("EngineSim", "Failed to initialize audio")
    return
}

// 스로틀 설정 (0.0 ~ 1.0)
engine.setThrottle(0.5f)

// 오디오 버퍼 읽기
val buffer = ShortArray(512)
val samplesRead = engine.readAudio(buffer, 512)

// RPM 조회
val rpm = engine.getRpm()
Log.d("EngineSim", "RPM: $rpm")

// 정리
engine.destroy()
```

## 빌드 방법 (Android Studio)

1. Android Studio에서 `android/` 폴더 열기
2. Gradle sync 실행
3. `Build > Make Project` 또는 `Run` (디바이스 연결)

### 빌드 산출물

- `app/.cxx/Debug/<abi>/libengine_sim_core.a` — 정적 라이브러리
- `app/build/intermediates/cmake/<abi>/obj/` — 오브젝트 파일

자세한 빌드 설정은 [docs/BUILD.md](docs/BUILD.md)를 참조하세요.

## 지원 환경

| 항목 | 최소 | 권장 |
|------|------|------|
| Android | API 26 (8.0) | API 34 |
| NDK | r21+ | r25+ |
| ABI | arm64-v8a | arm64-v8a, x86_64 |

## 라이선스

MIT License — 원본 engine-sim과 동일합니다.
자세한 내용은 [LICENSE](LICENSE) 파일을 참조하세요.

## 참고

이 프로젝트는 [bendens1/engine-sim](https://github.com/bendens1/engine-sim)을 기반으로 합니다.
원본 프로젝트의 저작물에 대한 모든 권리는 원저자에게 있습니다.