cmake_minimum_required(VERSION 3.11)

# C++17
set(CMAKE_CXX_STANDARD 17)

project(SmartCharsetConverter)

# 添加代码
file(GLOB COMMON_CODE
	src/Common/*.h
	src/Common/*.cpp
	src/Control/*.h
	src/Control/*.cpp
)

file(GLOB SRC_CODE
	src/*.h
	src/*.cpp
)

# 添加unicode宏
add_definitions(-DUNICODE -D_UNICODE)

# 添加include目录
include_directories(
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/third_party/WTL/include
	${PROJECT_SOURCE_DIR}/src/Common
	${PROJECT_SOURCE_DIR}/src/Control
)

# 添加uchardet库
add_subdirectory(${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet)
include_directories(${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet/src)

# 添加icu库
set(ICU_DIR "" CACHE STRING "icu include dir. e.g: D:\\icu which has several directories: include, bin64, lib64")
if (ICU_DIR STREQUAL "")
	message(FATAL_ERROR "must specify icu directory.")
endif()
if (NOT EXISTS ${ICU_DIR}/include)
	message(FATAL_ERROR "icu directory is wrong.")
endif()

include_directories(${ICU_DIR}/include)

file(GLOB ICU_LIBS ${ICU_DIR}/lib64/*.lib)
link_libraries(${ICU_LIBS})
# icu添加完成

find_package(boost)
if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIR})
else()
	message(FATAL_ERROR "没找到boost")
endif()

# 打印include目录情况
get_property(dirs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
message(">>> include_dirs=${dirs}")

# 目标exe
add_executable(${PROJECT_NAME} 
	${COMMON_CODE} 
	${SRC_CODE} 
	src/SmartCharsetConverter.cpp
)

# 添加.rc
target_sources(${PROJECT_NAME} PRIVATE src/SmartCharsetConverter.rc)

# 链接uchardet
target_link_libraries(${PROJECT_NAME} libuchardet_static)

# 完成后拷贝icu dll
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND 
		${CMAKE_COMMAND} -E copy	# CMAKE_COMMAND 代表cmake.exe
		${ICU_DIR}/bin64/icuuc71.dll
		${ICU_DIR}/bin64/icuin71.dll
		${ICU_DIR}/bin64/icudt71.dll
		$<TARGET_FILE_DIR:${PROJECT_NAME}>
)

# VS 设置 Subsystem 选项
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")