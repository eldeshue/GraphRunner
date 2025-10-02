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
FetchContent_MakeAvailable(googletest)

# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
)

# 테스트 바이너리 빌드에 필요한 라이브러리 링크 필요
# target_link_libraries(UnitTest PRIVATE 타겟1 타겟2 ... gtest gmock gtest_main)
enable_testing()
add_executable(UnitTest ${SRC_FILES})
target_link_libraries(UnitTest PRIVATE MyProject gtest gmock gtest_main)

include(GoogleTest)
gtest_discover_tests(UnitTest)
