# ============================================================
# Core Libraries
# ============================================================
add_library(Decoder-rv32i_lib STATIC rv32i.cc)

# ============================================================
# Header-only Libraries
# ============================================================

set(
  ALL_HERDERS
    Disasm_headers
)

foreach(header IN LISTS ALL_HERDERS)
  add_library(${header} INTERFACE)
  target_include_directories(
    ${header}
    INTERFACE
      "${PROJECT_SOURCE_DIR}/include"
  )
  target_compile_features(
    ${header}
    INTERFACE
      cxx_std_23
  )
  target_link_libraries(
    ${header}
    INTERFACE
      project_warnings
      project_sanitizers
  )
endforeach()


# ============================================================
# Common Static Library Settings
# ============================================================

set(
  ALL_LIBS
    Decoder-rv32i_lib
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

