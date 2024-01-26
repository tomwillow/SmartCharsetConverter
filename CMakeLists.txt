cmake_minimum_required(VERSION 3.11)

# C++17
set(CMAKE_CXX_STANDARD 17)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

project(SmartCharsetConverter)

# 添加unicode宏
add_definitions(-DUNICODE -D_UNICODE)

# 添加include目录
include_directories(
	${PROJECT_SOURCE_DIR}/third_party
	${PROJECT_SOURCE_DIR}/third_party/WTL/include
)

# 添加uchardet库
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet)

# 添加icu库
find_package(ICU REQUIRED
	COMPONENTS uc dt in io
)


find_package(GTest CONFIG REQUIRED)

# === CED ===
# 添加ced库

set(CED_ROOT "third_party/ced")
set(CED_LIBRARY_SOURCES
    ${CED_ROOT}/compact_enc_det/compact_enc_det.cc
    ${CED_ROOT}/compact_enc_det/compact_enc_det_hint_code.cc
    ${CED_ROOT}/util/encodings/encodings.cc
    ${CED_ROOT}/util/languages/languages.cc
    )

add_library(ced STATIC ${CED_LIBRARY_SOURCES})

target_include_directories(ced PUBLIC ${CED_ROOT})

# === Core ===
file(GLOB CORE_CODE
	src/Common/*.h
	src/Common/*.cpp
	src/Core/*.h
	src/Core/*.cpp
	)

add_library(Core STATIC ${CORE_CODE})

target_include_directories(Core PRIVATE
	${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet/src
)

target_link_libraries(Core PUBLIC
	libuchardet_static
	ICU::uc
	ICU::in
	ced
)

# 添加include目录
target_include_directories(Core PUBLIC
	${PROJECT_SOURCE_DIR}/src/Common
	${PROJECT_SOURCE_DIR}/src/Control
)

# === CoreUnitTest ===
set(SmartCharsetConverter_TEST_DIR "${PROJECT_SOURCE_DIR}/sample")
configure_file(src/test/config.h.in config.h @ONLY)
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

file(GLOB TEST_CODE
	src/test/*.h
	src/test/*.cpp
	)

enable_testing()
add_executable(CoreUnitTest ${TEST_CODE})
target_link_libraries(CoreUnitTest 
	PRIVATE
	GTest::gtest_main
	Core
	)
# 添加include目录
target_include_directories(CoreUnitTest PUBLIC
	${PROJECT_SOURCE_DIR}/src
)
	
include(GoogleTest)
gtest_discover_tests(CoreUnitTest)

# === guicon ============================
add_library(guicon STATIC
	${PROJECT_SOURCE_DIR}/third_party/guicon/guicon.h
	${PROJECT_SOURCE_DIR}/third_party/guicon/guicon.cpp
	)
	
# === SmartCharsetConverter =============

# 添加代码
file(GLOB COMMON_CODE
	src/Control/*.h
	src/Control/*.cpp
	src/ThreadPool/*.h
	src/ThreadPool/*.cpp
)

file(GLOB SRC_CODE
	src/*.h
	src/*.cpp
)

# 目标exe
add_executable(${PROJECT_NAME} 
	${COMMON_CODE} 
	${SRC_CODE} 
	src/SmartCharsetConverter.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	Core
	guicon
	)

# 添加include目录
target_include_directories(Core PRIVATE
	${PROJECT_SOURCE_DIR}/src
)

# 添加.rc
target_sources(${PROJECT_NAME} PRIVATE src/SmartCharsetConverter.rc)

# 完成后拷贝icu dll

# VS 设置 Subsystem 选项
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")