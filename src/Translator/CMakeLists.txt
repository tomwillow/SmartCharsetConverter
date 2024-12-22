# =============================================
find_package(nlohmann_json CONFIG REQUIRED)

# =======================================
file(GLOB_RECURSE SRC_CODE
	*.h
	*.cpp
)

add_library(Translator STATIC ${SRC_CODE})

target_link_libraries(Translator PUBLIC
	nlohmann_json::nlohmann_json
	Core
)

target_include_directories(Translator PUBLIC
	..
)
