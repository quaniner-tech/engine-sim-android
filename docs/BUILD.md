# Engine-Sim Android 빌드 가이드

## 환경 요구사항

| 항목 | 최소 | 권장 |
|------|------|------|
| Android Studio | 2022.2+ | 최신 |
| Android NDK | r21 | r25+ |
| CMake | 3.18+ | 3.22+ |
| JDK | 11 | 17 |
| Android SDK | API 26 | API 34 |

---

## 1. Android Studio에서 빌드

### 프로젝트 열기

1. Android Studio 실행
2. `File > Open` → `engine-sim-android/android/` 폴더 선택
3. Gradle sync 완료 대기

### 빌드 실행

```bash
# Debug 빌드 (CLI)
cd android && ./gradlew assembleDebug

# Release 빌드
./gradlew assembleRelease
```

### 빌드 산출물

```
android/app/build/outputs/
├── aar/                          # AAR 라이브러리
│   └── app-debug.aar
├── apk/
│   └── app-debug.apk
└── cmake/
    └── <abi>/                    # 네이티브 라이브러리
        └── libengine_sim_core.a
```

---

## 2. NDK 및 CMake 설정

### local.properties 설정

```properties
sdk.dir=/path/to/Android/Sdk
ndk.dir=/path/to/Android/Sdk/ndk/25.0.000000000
```

### build.gradle (app)

```gradle
android {
    defaultConfig {
        ndk {
            abiFilters += ["armeabi-v7a", "arm64-v8a", "x86", "x86_64"]
        }

        externalNativeBuild {
            cmake {
                cppFlags "-std=c++17 -fno-rtti -fno-exceptions"
                arguments += [
                    "-DANDROID_STL=c++_static",
                    "-DCMAKE_POSITION_INDEPENDENT_CODE=ON"
                ]
            }
        }
    }

    buildFeatures {
        prefab true  // 메이븐 배포 시 필요
    }
}
```

---

## 3. AAR 라이브러리 배포 (Maven Local)

### 사전 요구사항

- Android Studio 또는 Gradle 8.x
- `android/gradle.properties`에 signing 설정 (릴리스용)

### 배포手順

#### 3-1. lib 모듈 생성

```
android/lib/
├── build.gradle              # lib 전용 빌드 설정
├── proguard-rules.pro        # consumer ProGuard 규칙
└── src/main/
    └── (lib의 소스 또는 AAR 원본)
```

#### 3-2. Gradle Publishing 설정

`android/lib/build.gradle`에 다음 추가:

```gradle
plugins {
    id 'com.android.library'
    id 'maven-publish'
}

publishing {
    publications {
        aar(MavenPublication) {
            groupId = 'com.enginesim'
            artifactId = 'engine-sim-android'
            version = '1.0.0'

            from components.aar

            pom {
                name = 'Engine-Sim Android Library'
                description = 'Engine simulation for Android with JNI'
                licenses {
                    license {
                        name = 'MIT License'
                        url = 'https://opensource.org/licenses/MIT'
                    }
                }
            }
        }
    }
}
```

#### 3-3. 배포 실행

```bash
# Maven Local에 배포
cd android
./gradlew :lib:publishAarPublicationToMavenLocal

# 출력 위치
# ~/.m2/repository/com/enginesim/engine-sim-android/1.0.0/
```

#### 3-4. Consumer 앱에서 사용

```kotlin
// settings.gradle
dependencyResolutionManagement {
    mavenLocal()
    repositoriesMode.set(RepositoriesMode.PREFER_SETTINGS)
}

// app/build.gradle
dependencies {
    implementation("com.enginesim:engine-sim-android:1.0.0")
}
```

---

## 4. 네이티브 라이브러리 수동 빌드 (NDK)

### CMakeLists.txt (네이티브 빌드용)

```cmake
cmake_minimum_required(VERSION 3.18)
project(engine_sim_android)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(${CMAKE_SOURCE_DIR}/../../core engine_sim_core_build)

set(NATIVE_SOURCES
    cpp/aaudio_engine.cpp
    cpp/android_audio_output.cpp
    cpp/JniBridge.cpp
)

add_library(engine_sim_jni SHARED ${NATIVE_SOURCES})

target_include_directories(engine_sim_jni PRIVATE
    ${CMAKE_SOURCE_DIR}/../../core/include
    cpp
)

target_link_libraries(engine_sim_jni
    engine_sim_core
    android
    log
)
```

### 빌드 명령

```bash
# NDK 경로 확인
export ANDROID_NDK_HOME=/path/to/Android/Sdk/ndk/25.0.000000000

# ABI 지정 빌드
cd android
./gradlew :app:externalNativeBuildDebug -PABI=arm64-v8a

# 또는 cmake 직접 호출
mkdir -p build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-26 \
      ../app/src/main/cpp
make
```

---

## 5. 문제 해결

### CMake 오류: "CMAKE_CXX_COMPILER not set"

```bash
# NDK가 제대로 설치되어 있는지 확인
ls $ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin/

# cmake 설정
-DANDROID_NDK=$ANDROID_NDK_HOME
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake
```

### linking 오류: undefined reference to 'AAudio'

```gradle
// NDK r21+에서는 AAudio가 libc++에 포함됨
// 별도 링크 필요 없음 (build.gradle의 android 라이브러리만으로 충분)
target_link_libraries(engine_sim_jni android log)
```

### AAR 탐지 안 됨 (Prefabrication 오류)

```gradle
// app/build.gradle
android {
    buildFeatures { prefab = true }
}

android.library {
    // library는 기본적으로 prefab 활성화
}
```

### Gradle sync 실패: NDK not found

```properties
# gradle.properties
android.ndkVersion=25.0.000000000
```

---

## 6. 성능 최적화

### 빌드 속도 향상

```bash
# 병렬 빌드
./gradlew assembleDebug -Pandroid.buildCacheEnabled=true -Porg.gradle.parallel=true

# C++ 캐시 사용
./gradlew :app:configureCMakeRelWithDebInfo -Pandroid.enableCcache=true
```

### 런타임 성능

| 항목 | 권장 값 | 설명 |
|------|---------|------|
| 오디오 버퍼 | 512 frames | 지연 최소화 |
| Ring buffer | 16384 frames | 언더런 방지 |
| 오디오 스레드 | 별도 코루틴 | UI 블로킹 방지 |
| NDK abiFilter | arm64-v8a only | 디바이스에 맞게 |