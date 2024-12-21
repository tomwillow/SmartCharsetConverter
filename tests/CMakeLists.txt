find_package(GTest CONFIG REQUIRED)

enable_testing()

# === CoreUnitTest ===
file(GLOB TEST_CODE
	*.h
	*.cpp
)

add_executable(CoreUnitTest ${TEST_CODE})

target_link_libraries(CoreUnitTest
	PRIVATE
	GTest::gtest_main
	Core
)

# ===========================================
set(SmartCharsetConverter_TEST_DIR u8\"${PROJECT_SOURCE_DIR}/sample\")
configure_file(config.h.in config.h @ONLY)
target_include_directories(CoreUnitTest PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

# ===========================================
include(GoogleTest)
gtest_discover_tests(CoreUnitTest)
