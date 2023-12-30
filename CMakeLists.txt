cmake_minimum_required(VERSION 3.11)

# C++17
set(CMAKE_CXX_STANDARD 17)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

project(SmartCharsetConverter)
	
# === SmartCharsetConverter =============

# 添加代码
file(GLOB COMMON_CODE
	src/Common/*.h
	src/Common/*.cpp
	src/Control/*.h
	src/Control/*.cpp
	src/ThreadPool/*.h
	src/ThreadPool/*.cpp
)

file(GLOB SRC_CODE
	src/*.h
	src/*.cpp
)

# 添加unicode宏
add_definitions(-DUNICODE -D_UNICODE)

# 目标exe
add_executable(${PROJECT_NAME} 
	${COMMON_CODE} 
	${SRC_CODE} 
	src/SmartCharsetConverter.cpp
)

# 添加include目录
target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/third_party
	${PROJECT_SOURCE_DIR}/third_party/WTL/include
	${PROJECT_SOURCE_DIR}/src/Common
	${PROJECT_SOURCE_DIR}/src/Control
)

# === guicon ============================
add_library(guicon STATIC
	${PROJECT_SOURCE_DIR}/third_party/guicon/guicon.h
	${PROJECT_SOURCE_DIR}/third_party/guicon/guicon.cpp
	)

# 添加uchardet库
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet)
target_include_directories(${PROJECT_NAME} PRIVATE
	${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet/src
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	libuchardet_static
	guicon
)

# 添加icu库
find_package(ICU REQUIRED
	COMPONENTS uc dt in io
)
target_link_libraries(${PROJECT_NAME} PRIVATE
	ICU::uc
	ICU::in
)

# add nlohmann/json library
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

# 添加.rc
target_sources(${PROJECT_NAME} PRIVATE src/SmartCharsetConverter.rc)

# 完成后拷贝icu dll


# VS 设置 Subsystem 选项
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")