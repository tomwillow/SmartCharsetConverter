# === SmartCharsetConverter-imgui =============
set(target_name SmartCharsetConverter-imgui)

file(GLOB SRC_CODE
	*.h
	*.cpp
)

# 目标exe
add_executable(${target_name}
	${SRC_CODE}
	SmartCharsetConverter.rc
)

find_package(imgui CONFIG REQUIRED)

target_link_libraries(${target_name} PRIVATE
	Core
	Translator
	guicon
	imgui::imgui
	opengl32.lib
)

# 添加include目录
target_include_directories(${target_name} PRIVATE
	.
)

# VS 设置 Subsystem 选项
set_target_properties(${target_name} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
set_target_properties(${target_name} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
set_target_properties(${target_name} PROPERTIES LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")