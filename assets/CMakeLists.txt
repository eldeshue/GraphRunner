# ====================================================================
# GraphRunner Asset Processing
# ====================================================================

cmake_minimum_required(VERSION 3.23)

# ====================================================================
# 1. Basic Path Setting
# ====================================================================

# Original Asset path (이 파일이 있는 위치)
set(SOURCE_ASSET_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# After-build Asset path
# 빌드 후 '실행 파일'이 참조할 최종 에셋 출력 디렉터리
# 예: build/data/
# (example/my_example.exe 에서는 "../data/" 경로로 접근하게 됩니다)
set(RUNTIME_ASSET_DIR "${CMAKE_BINARY_DIR}/data")

# ====================================================================
# 2. Compile Shaders 
# ====================================================================

# VULKAN_SDK 환경 변수 경로 힌트
set(VULKAN_SDK_BIN_HINT "$ENV{VULKAN_SDK}/Bin")

# GLSL 컴파일러 (glslangValidator)
# OS에 따라 .exe를 자동으로 처리합니다.
find_program(GLSLANG_VALIDATOR_EXECUTABLE glslangValidator
    HINTS ${VULKAN_SDK_BIN_HINT}
    DOC "path for glslangValidator executable"
)
if(NOT GLSLANG_VALIDATOR_EXECUTABLE)
    message(WARNING "glslangValidator not found, cannot compile GLSL shaders. Check VulkanSDK.")
endif()
message(STATUS "glslangValidator found: ${GLSLANG_VALIDATOR_EXECUTABLE}")

# find shader source files
file(GLOB_RECURSE SHADER_SOURCE_FILES
    CONFIGURE_DEPENDS
    "${SOURCE_ASSET_DIR}/*.glsl"
    "${SOURCE_ASSET_DIR}/*.vert"
    "${SOURCE_ASSET_DIR}/*.tesc"
    "${SOURCE_ASSET_DIR}/*.tese"
    "${SOURCE_ASSET_DIR}/*.geom"
    "${SOURCE_ASSET_DIR}/*.frag"
    "${SOURCE_ASSET_DIR}/*.comp"
    # for raytracing
    "${SOURCE_ASSET_DIR}/*.rgen"
    "${SOURCE_ASSET_DIR}/*.rint"
    "${SOURCE_ASSET_DIR}/*.rahit"
    "${SOURCE_ASSET_DIR}/*.rchit"
    "${SOURCE_ASSET_DIR}/*.rmiss"
    "${SOURCE_ASSET_DIR}/*.rcall"
)

set(COMPILED_SPV_FILES "")

# 2-3. 각 셰이더 파일에 대해 컴파일러 선택 및 커맨드 생성
foreach(SHADER_SRC_FILE ${SHADER_SOURCE_FILES})
    
    # 출력 경로 계산 (예: .../build/data/shaders/lit.frag.spv)
    string(REPLACE "${SOURCE_ASSET_DIR}" "${RUNTIME_ASSET_DIR}" 
                   SPV_DST_FILE_BASE ${SHADER_SRC_FILE})
    set(SPV_DST_FILE "${SPV_DST_FILE_BASE}.spv")
    get_filename_component(SPV_DST_DIR ${SPV_DST_FILE} DIRECTORY)

    # 셰이더 확장자에 따라 컴파일러 선택
    string(TOLOWER "${SHADER_SRC_FILE}" SHADER_SRC_LOWER)
    set(COMPILER_COMMAND "")
    set(COMPILER_FOUND TRUE)

    if(SHADER_SRC_LOWER MATCHES "\\.(glsl|vert|tesc|tese|geom|frag|comp|rgen|rint|rahit|rchit|rmiss|rcall)$")
        if(NOT GLSLANG_VALIDATOR_EXECUTABLE)
            set(COMPILER_FOUND FALSE)
        else()
            # GLSL -> glslang
            # -V: Vulkan 시맨틱으로 컴파일
            # glslang은 .vert, .frag 등의 확장자로 셰이더 스테이지를 자동 감지합니다.
            set(COMPILER_COMMAND
                COMMAND ${GLSLANG_VALIDATOR_EXECUTABLE} -V -o ${SPV_DST_FILE} ${SHADER_SRC_FILE}
            )
        endif()
        # hlsl을 지원하기 위해서는 각 shader에 대해서 stage와 entry point를 명시해줘야 함.
        # 현재는 hlsl을 지원하지 않음.
    endif()

    # 2-4. 증분 컴파일 커맨드 정의 (컴파일러를 찾은 경우에만)
    if(COMPILER_FOUND AND NOT COMPILER_COMMAND STREQUAL "")
        add_custom_command(
            OUTPUT  ${SPV_DST_FILE}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${SPV_DST_DIR}
            ${COMPILER_COMMAND} # 선택된 컴파일러 명령 사용
            DEPENDS ${SHADER_SRC_FILE}
            COMMENT "Compiling shader: ${SHADER_SRC_FILE}"
        )
        list(APPEND COMPILED_SPV_FILES ${SPV_DST_FILE})
    endif()
endforeach()

# 2-5. 셰이더 컴파일 더미 타겟 (이전과 동일)
add_custom_target(CompileShaders
    DEPENDS ${COMPILED_SPV_FILES}
    COMMENT "Compiling all modified shaders"
)

# ====================================================================
# 3. Copy Resources
# ====================================================================

# copy textures, compiled shaders, mesh objects, etc
file(GLOB_RECURSE ALL_ASSET_FILES
    CONFIGURE_DEPENDS
    "${SOURCE_ASSET_DIR}/*"
)
list(REMOVE_ITEM ALL_ASSET_FILES ${SHADER_SOURCE_FILES})    # exclude shader source files
set(COPIED_RESOURCE_FILES "")
foreach(RESOURCE_SRC_FILE ${ALL_ASSET_FILES})
    string(REPLACE "${SOURCE_ASSET_DIR}" "${RUNTIME_ASSET_DIR}"
                   RESOURCE_DST_FILE ${RESOURCE_SRC_FILE})
    get_filename_component(RESOURCE_DST_DIR ${RESOURCE_DST_FILE} DIRECTORY)
    add_custom_command(
        OUTPUT  ${RESOURCE_DST_FILE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${RESOURCE_DST_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${RESOURCE_SRC_FILE} ${RESOURCE_DST_FILE}
        DEPENDS ${RESOURCE_SRC_FILE}
        COMMENT "Copying resource: ${RESOURCE_SRC_FILE}"
    )
    list(APPEND COPIED_RESOURCE_FILES ${RESOURCE_DST_FILE})
endforeach()
add_custom_target(CopyResources
    DEPENDS ${COPIED_RESOURCE_FILES}
    COMMENT "Copying all modified resources"
)

# ====================================================================
# 4. Master Target (AssetProcessing)
# ====================================================================
add_custom_target(AssetProcessing
    DEPENDS
        CompileShaders
        CopyResources
    COMMENT "Processing all engine assets"
)