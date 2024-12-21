# ==============================
file(GLOB_RECURSE SRC_CODE
	*.h
	*.cpp
	)

add_library(Common STATIC ${SRC_CODE})
