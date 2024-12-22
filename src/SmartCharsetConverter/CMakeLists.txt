# === SmartCharsetConverter =============

file(GLOB SRC_CODE
	*.h
	*.cpp
)

# 目标exe
add_executable(${PROJECT_NAME}
	${SRC_CODE}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
	Core
	Control
	Translator
	guicon
)

# 添加include目录
target_include_directories(Core PRIVATE
	.
)

# 添加.rc
target_sources(${PROJECT_NAME} PRIVATE SmartCharsetConverter.rc)

# VS 设置 Subsystem 选项
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")