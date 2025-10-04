cmake_minimum_required(VERSION 3.23)

# 현재 디렉토리에 있는 모든 파일을 SRC_FILES 변수에 추가한다.
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/*.cc
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
        ${CMAKE_CURRENT_SOURCE_DIR}/lib    # 내부 전용 헤더
)

# 현재 타겟에 대한 링킹 추가
# lib 아래에 추가한 라이브러리를 여기에 추가
# 만약 vcpkg를 통해서 관리될 라이브러리의 경우에는 find_package 필요
# find_package(패키지 이름 CONFIG REQUIRED)
# target_link_libraries(타겟이름 PUBLIC 링크할라이브러리이름)


# MSVC 컴파일러를 사용할 때만 Microsoft Code Analysis를 활성화합니다.
if(MSVC)
    target_compile_options(GraphRunner PRIVATE /analyze-)
    message(STATUS "Microsoft Code Analysis (/analyze) enabled for MSVC builds.")
endif()

