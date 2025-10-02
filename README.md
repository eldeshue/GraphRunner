# Simple cmake template
## Description
cmake를 사용한 간다한 c++프로젝트를 위한 템플릿.

일단은 linux, mac, windows의 multi-platform을 목적으로 하지만, MSVC에서는 검증이 필요함.

## Directory Setting
- `src/` : 프로젝트를 구성하는 실제 구현 파일을 위한 경로.
- `include/` : 프로젝트를 구성하는 헤더를 위한 겨올. 라이브러리 프로젝트라면 외부에 노출시킬 header를 여기에.
- `lib/` : 프로젝트에서 별도로 분리, linking될 라이브러리를 위한 경로.
- `tests/` : GoogleTest를 활용한 test code를 위한 경로.
- `example/` : 라이브러리 프로젝트에서 해당 라이브러리의 사용법을 위한 경로.
- `out/` : 빌드 구성 및 빌드 결과를 위한 경로.

# How to use?
## Project Setting
repo 생성 과정에서 해당 template를 base로 선택. repo가 생성되면 clone한 후 다음을 따른다.
root의 CMakeLists.txt에 project 이름 및 프로젝트 옵션을 설정한다.
이후 프로젝트의 목적에 따라 다음의 두 과정 중 하나를 선택한다.

### 1. Binary Project
프로젝트가 실행 가능한 binary의 생성을 목표로 하는 경우
1. `example/`, `tests/`등 라이브러리를 위한 디렉토리 삭제
2. 프로젝트의 root CMakeLists.txt 수정
	- add_sub_dir로 tests와 example을 추가하는 것을 막아야 함
3. src/CMakeLists.txt 수정
	- add_executable 설정
	- 의존성을 위해서 target_link_library 설정
	- add_library, target_include_directory 삭제

### 2. Library Project
프로젝트가 정적 라이브러리의 생성을 목표로 하는 경우
1. 프로젝트의 root CMakeLists.txt 수정
2. src/CMakeLists.txt 수정
	- add_library, target_include_directory 설정
	- 의존성을 위해서 target_link_library 설정하기
	- add_executable 삭제

## 라이브러리 추가하기
개발 과정에서 .lib 아래에 라이브러리를 추가하여 기능을 분리하는 경우.
1. `lib/` 아래에 프로젝트 추가
2. `lib/CMakeLists.txt`에 추가한 디렉토리에 대하여 add_subdirectory 추가

# 🛠️ 프로젝트 빌드 & 실행 가이드 (CMake Presets 기반)
이 프로젝트는 CMakePresets.json을 활용하여 다양한 플랫폼 및 빌드 설정을 지원합니다.

## 📦 1. 구성(Configure)
```bash
cmake --preset <preset-name>
```
예시:
- Windows + MSVC Debug (Developer Mode):

```bash
cmake --preset windows-msvc-debug-developer-mode
```
- Unix-like + Clang Release:

```bash
cmake --preset unixlike-clang-release
```
## 🧱 2. 빌드(Build)
``` bash
cmake --build --preset <preset-name>
```

예시:

``` bash
cmake --build --preset windows-msvc-debug-developer-mode
```

## 🧪 3. 테스트(Test)
``` bash
ctest --preset <test-preset-name>
```

예시:

``` bash
ctest --preset test-unixlike-gcc-debug
```

※ 실패한 테스트에서 자세한 로그를 출력하며, 실패 시 즉시 중단됩니다.

## 🐞 4. 디버깅(Debug)
빌드된 바이너리는 다음 경로에 위치합니다:

```bash
out/build/<preset-name>/bin/
```
예를 들어:

```bash
./out/build/unixlike-gcc-debug/bin/my_executable
./out/build/unixlike-gcc-debug/bin/test_myproject
```
VSCode에서는 .vscode/launch.json을 구성하여 디버깅 가능합니다.

# ⚙️ 주요 Presets 요약
Preset 이름	설명
- `windows-msvc-debug-developer-mode` :	Windows + MSVC + Debug + 개발자 모드
- `windows-clang-release` :	Windows + Clang + Release
- `unixlike-gcc-debug` :	Linux/macOS + GCC + Debug
- `unixlike-clang-release` :	Linux/macOS + Clang + Release

Test Presets는 위의 configure preset 이름 앞에 test-를 붙이면 됩니다.
예: test-unixlike-clang-release

# 📁 빌드 아티팩트 구조
디렉토리	설명
- `out/build/<preset-name>` :	CMake 빌드 디렉토리
- `out/build/<preset-name>/bin/` :	실행 파일 및 테스트 바이너리 위치
- `out/install/<preset-name>` :	설치 디렉토리 (선택적으로 사용 가능)

# 🚧 주의
CMake ≥ 3.23이 필요합니다.

VSCode를 사용하는 경우 자동으로 preset을 인식하며, "CMake: Configure"를 실행하면 선택 가능.

테스트 실행 전 반드시 빌드가 완료되어야 합니다.
