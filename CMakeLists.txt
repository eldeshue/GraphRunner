# Root CMake for the project
cmake_minimum_required(VERSION 3.23)

# project info
project(
  GraphRunner
  VERSION 0.1
  DESCRIPTION "GraphRunner: A modern, cross-platform 3D engine."
  LANGUAGES CXX)

# 컴파일 관련 설정 외부로 노출(CMake에 연동된 다른 기능이 참조함)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_subdirectory(editor)	# editor, running binary for GraphRunnerEngine

# If MSVC is being used, and ASAN is enabled, we need to set the debugger environment
# so that it behaves well with MSVC's debugger, and we can run the target from visual studio
if(MSVC)
  # add executable as a targets
  # set_target_properties(실행1 실행2 ... 실행n PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
  set_target_properties(example UnitTest PROPERTIES VS_DEBUGGER_ENVIRONMENT "PATH=$(VC_ExecutablePath_x64);%PATH%")
endif()


# ===================================================================
# Deactivate Clang-Tidy for Google Test
# ===================================================================
# gtest, gmock, gtest_main 타겟이 생성된 후에 속성을 덮어쓴다.
set_target_properties(gtest gmock gtest_main PROPERTIES
  CXX_CLANG_TIDY "" # 빈 문자열로 설정하여 Clang-Tidy를 끔
)

# ===================================================================
# ClangFormat integration
# ===================================================================

# 1. find clang-format executable
find_program(CLANG_FORMAT_EXECUTABLE clang-format)

# 2. check existence of clang-format
if(CLANG_FORMAT_EXECUTABLE)
    # 3. make list of target files to apply format
    #    (recursively searching .h, .cpp files)
    file(GLOB_RECURSE FORMATTING_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/editor/*.h"
        "${PROJECT_SOURCE_DIR}/editor/*.cc"
    )

    message(STATUS "Found files for formatting:")
	foreach(source_file IN LISTS FORMATTING_SOURCES)
		message(STATUS "  - ${source_file}")
	endforeach()

    # 4. add custom target named 'format'
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXECUTABLE}
                -i # direct modification (in-place)
                --style=file # style based on .clang-format file
                ${FORMATTING_SOURCES} # target list
        
        COMMENT "Formatting C++ source files with clang-format..."
        
        # 다른 소스 파일이 변경되어도 항상 실행되도록 설정
        VERBATIM
    )
    
    # in case failed to find clang-format
else()
    message(WARNING "clang-format not found. The 'format' target will not be available.")
endif()

# ===================================================================
# ClangTidy integration (Custom Target)
# ===================================================================

# 1. find clang-tidy executable
find_program(CLANG_TIDY_EXECUTABLE clang-tidy)

# 2. check existence of clang-tidy
if(CLANG_TIDY_EXECUTABLE)
    # 3. make list of target files to apply tidy check
    #    (recursively searching source files)
    file(GLOB_RECURSE TIDY_SOURCES
        "${PROJECT_SOURCE_DIR}/src/*.cc"
        "${PROJECT_SOURCE_DIR}/src/*.cpp"
        "${PROJECT_SOURCE_DIR}/include/*.h"
        "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/editor/*.h"
        "${PROJECT_SOURCE_DIR}/editor/*.cc"
        # ... 프로젝트에 포함된 모든 소스 파일 경로 패턴 추가 ...
    )

    # 4. add custom target named 'clang-tidy'
    add_custom_target(clang-tidy
        COMMAND ${CLANG_TIDY_EXECUTABLE}
                -p ${CMAKE_BINARY_DIR}
                ${TIDY_SOURCES}
        
        COMMENT "Running clang-tidy on all sources..."
        VERBATIM
    )

# in case failed to find clang-tidy
else()
    message(WARNING "clang-tidy not found. The 'clang-tidy' target will not be available.")
endif()
