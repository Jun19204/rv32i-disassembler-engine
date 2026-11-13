# ============================================================
# Core Libraries
# ============================================================
add_library(Decoder-rv32i_lib STATIC rv32i.cc)
add_library(Parser-elf_parser_lib STATIC elf_parser.cc)

# ============================================================
# Common Static Library Settings
# ============================================================

set(
  ALL_LIBS
    Decoder-rv32i_lib
    Parser-elf_parser_lib
)

foreach(lib IN LISTS ALL_LIBS)
  target_include_directories(
    ${lib}
    PUBLIC
      "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_features(
    ${lib}
    PUBLIC
      cxx_std_23
  )
  target_link_libraries(
    ${lib}
    PRIVATE
      project_warnings
      project_sanitizers
  )
endforeach()

