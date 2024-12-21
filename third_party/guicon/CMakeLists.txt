# ========================
file(GLOB SRC_CODE
	*.h
	*.cpp
	)

add_library(guicon STATIC ${SRC_CODE})

# 添加include目录
target_include_directories(guicon PUBLIC
	..
)
