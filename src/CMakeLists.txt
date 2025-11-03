cmake_minimum_required(VERSION 3.23)

# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# 프로젝트가 실행 바이너리
# add_executable(타겟실행파일이름 ${SRC_FILES})

# 프로젝트가 정적 라이브러리
# add_library(타겟라이브러리이름 STATIC ${SRC_FILES})
# target_include_directories(타겟라이브러리이름
#     PUBLIC
#         ${PROJECT_SOURCE_DIR}/include      # 공개 헤더 (API)
#     PRIVATE
#         ${CMAKE_CURRENT_SOURCE_DIR}/lib    # 내부 전용 헤더
# )
add_library(GraphRunner STATIC ${SRC_FILES})
target_include_directories(GraphRunner
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include      # 공개 헤더 (API)
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Util   # Utilities 
        ${CMAKE_CURRENT_SOURCE_DIR}/Rhi    # RHI
    SYSTEM BEFORE
        ${PROJECT_SOURCE_DIR}/lib    # 내부 전용 헤더
)

# add compile options for the project
target_compile_options(GraphRunner PRIVATE
    # MSVC (Visual Studio)일 경우 예외 처리 모델 활성화
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>
    
    # GNU(GCC) 또는 Clang 컴파일러일 경우 예외 활성화
    $<$<CXX_COMPILER_ID:GNU>:-fexceptions>
    $<$<CXX_COMPILER_ID:Clang>:-fexceptions>

    # (여기에 다른 플래그들을 추가할 수 있습니다)
)

###################################################################
# 현재 타겟에 대한 링킹 추가
# lib 아래에 추가한 라이브러리를 여기에 추가
# 만약 vcpkg를 통해서 관리될 라이브러리의 경우에는 find_package 필요
# find_package(패키지 이름 CONFIG REQUIRED)
# target_link_libraries(타겟이름 PUBLIC 링크할라이브러리이름)

##################################################################
# vulkan sdk 정적 linking
# SDK 경로를 변수로 설정 (알고 계신 경로를 여기에 입력)
set(VULKAN_SDK_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
set(VULKAN_SDK_LIBRARY_DIR "$ENV{VULKAN_SDK}/Lib")

target_include_directories(GraphRunner PUBLIC SYSTEM
    ${VULKAN_SDK_INCLUDE_DIR}
)

# 링크할 라이브러리 파일의 전체 경로 찾기
#    (예: libfoo.a 또는 foo.lib의 경우 "foo"를 입력)
# find_library(
#     MY_SDK_LIBRARY_FILE  # 이 변수에 라이브러리 전체 경로가 저장됩니다.
#     NAMES a_lib_name     # 찾으려는 라이브러리 이름 (예: "foo", "bar")
#     HINTS ${MY_SDK_LIBRARY_DIR} # 이 경로를 우선적으로 검색
# )


# volk, vulkan loader
# instead of linking vulkan, link volk
find_library(
    VOLK 
    NAMES volk     # 찾으려는 라이브러리 이름 (예: "foo", "bar")
    HINTS ${VULKAN_SDK_LIBRARY_DIR} # 이 경로를 우선적으로 검색
)
if(WIN32)
    set(VOLK_STATIC_DEFINES  VK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_MACOS_MVK)
    # Link against Metal framework for MoltenVK
    target_link_libraries(GraphRunner PRIVATE "-framework Metal" "-framework MetalKit" "-framework Cocoa")
elseif(UNIX AND NOT APPLE)
    # Linux-specific configuration
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XCB_KHR)
    # You might need to link against XCB or Wayland libraries
endif()
target_link_libraries(GraphRunner PRIVATE ${VOLK})

###################################################################

# MSVC 컴파일러를 사용할 때만 Microsoft Code Analysis를 활성화합니다.
if(MSVC)
    target_compile_options(GraphRunner PRIVATE /analyze-)
    message(STATUS "Microsoft Code Analysis (/analyze) enabled for MSVC builds.")
endif()

# Vulkan validation layer enable
option(VULKAN_VALIDATION "Vulkan validation layers" OFF)
if(VULKAN_VALIDATION)
    target_compile_definitions(GraphRunner PRIVATE ENABLE_VULKAN_VALIDATION)
    message(STATUS "Vulkan validation layers: ON")
else()
    message(STATUS "Vulkan validation layers: OFF")
endif()

# Vulkan Portability extension enable
option(VULKAN_PORTABILITY "Vulkan portability extension" OFF)
if(VULKAN_PORTABILITY)
    target_compile_definitions(GraphRunner PRIVATE ENABLE_VULKAN_PORTABILITY)
    message(STATUS "Vulkan portability layers: ON")
else()
    message(STATUS "Vulkan portability layers: OFF")
endif()

