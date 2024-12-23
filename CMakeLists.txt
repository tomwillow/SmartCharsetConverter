cmake_minimum_required(VERSION 3.11)

# C++17
set(CMAKE_CXX_STANDARD 17)

project(SmartCharsetConverter)

# 添加unicode宏
add_definitions(-DUNICODE -D_UNICODE)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options(-WX)

if(MSVC)
	add_compile_options(/MP)

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MT")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")

    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")

    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# =============================================
add_subdirectory(third_party)
add_subdirectory(src)
add_subdirectory(tests)
