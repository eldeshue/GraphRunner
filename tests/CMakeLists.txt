cmake_minimum_required(VERSION 3.23)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(gtest_build_with_exceptions ON CACHE BOOL "Force gtest to build with exceptions" FORCE)
FetchContent_MakeAvailable(googletest)

# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
# file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
#  ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
# )

# 테스트 바이너리 빌드에 필요한 라이브러리 링크 필요
# target_link_libraries(UnitTest PRIVATE 타겟1 타겟2 ... gtest gmock gtest_main)
enable_testing()
add_executable(RhiTest RhiTest.cc)
target_link_libraries(RhiTest PRIVATE GraphRunner gtest gmock gtest_main)
# add compile options for the project
target_compile_options(RhiTest PRIVATE
    # MSVC (Visual Studio)일 경우 예외 처리 모델 활성화
    "$<IF:$<CXX_COMPILER_ID:MSVC>,
        /EHsc
    , 0>"

    # GNU(GCC) 또는 Clang 컴파일러일 경우 예외 활성화
    "$<IF:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>,
        -fexceptions
    , 0>"

    # (여기에 다른 플래그들을 추가할 수 있습니다)
    # 예: "$<IF:$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>, -Wall, 0>"
)

include(GoogleTest)
gtest_discover_tests(RhiTest)
