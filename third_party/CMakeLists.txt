# === CED ===
# 添加ced库
set(CED_ROOT "ced")
set(CED_LIBRARY_SOURCES
    ${CED_ROOT}/compact_enc_det/compact_enc_det.cc
    ${CED_ROOT}/compact_enc_det/compact_enc_det_hint_code.cc
    ${CED_ROOT}/util/encodings/encodings.cc
    ${CED_ROOT}/util/languages/languages.cc
)

add_library(ced STATIC ${CED_LIBRARY_SOURCES})

target_include_directories(ced PUBLIC ${CED_ROOT})

# =====================================
add_subdirectory(uchardet/uchardet)
add_subdirectory(guicon)