# ==============================
file(GLOB_RECURSE SRC_CODE
	*.h
	*.cpp
	)

add_library(Control STATIC ${SRC_CODE})

# 添加include目录
target_include_directories(Control PUBLIC
	..
	${PROJECT_SOURCE_DIR}/third_party/WTL/include
)
