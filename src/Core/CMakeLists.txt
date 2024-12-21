# =============================================
find_package(nlohmann_json CONFIG REQUIRED)

# 添加icu库
find_package(ICU REQUIRED
	COMPONENTS uc dt in io data
)

# === Core ====================================
file(GLOB CORE_CODE
	*.h
	*.cpp
)

add_library(Core STATIC ${CORE_CODE})

target_include_directories(Core PRIVATE
	${PROJECT_SOURCE_DIR}/third_party/uchardet/uchardet/src
)

target_link_libraries(Core PUBLIC
	Common
	nlohmann_json::nlohmann_json
	libuchardet_static
	ICU::uc
	ICU::in
	ICU::data
	ced
) # 添加include目录
target_include_directories(Core PUBLIC
	..
)
