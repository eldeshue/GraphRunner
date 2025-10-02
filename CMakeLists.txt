# Root CMake for the project
cmake_minimum_required(VERSION 3.23)

# project info
project(
  DefaultName
  VERSION 0.1
  DESCRIPTION "Default cmake project description."
  LANGUAGES CXX)

# C++ compiler setting
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 라이브러리 파일은 빌드 디렉토리 안에 lib 폴더에 출력.
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 실행 파일은 빌드 디렉토리 안에 bin 폴더에 출력.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 외부에 노출하거나 프로젝트가 포함할 라이브러리의 헤더 경로
include_directories(${PROJECT_SOURCE_DIR}/include)

# activate ctest
enable_testing()

# sub dir path setting
# sub dir must have it's own cmake
add_subdirectory(src)		# actual implementation
add_subdirectory(lib)		# actual implementation

# Don't even look at tests if we're not top level
if(NOT PROJECT_IS_TOP_LEVEL)
  return()
endif()

# set the startup project for the "play" button in MSVC
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT intro)

# 현재 프로젝트가 binary project라면, 이하를 삭제.
add_subdirectory(tests)		# test framework, GoogleTest
add_subdirectory(example)	# example, show usage of the library

# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if(MSVC)
  # add executable as a targets
  # set_target_properties(실행1 실행2 ... 실행n PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
  set_target_properties(example UnitTest PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()


