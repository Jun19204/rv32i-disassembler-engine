# ============================================================
# 1. Add Executables (실행 파일 타겟 등록)
# ============================================================
add_executable(disasm-rv32i disasm-rv32i.cc)

# ============================================================
# 2. Link Libraries (개별 라이브러리 및 헤더 경로 연결)
# ============================================================
target_link_libraries(disasm-rv32i 
  PRIVATE 
    Decoder-rv32i_lib
    Parser-elf_parser_lib
)


# ===========================================================
# 3. Common Settings (C++23 표준 및 공통 플래그 일괄 주입)
# ===========================================================
set(
  ALL_EXES
    disasm-rv32i
)

foreach(exe IN LISTS ALL_EXES)
  target_compile_features(${exe} PRIVATE cxx_std_23)
  
  target_link_libraries(
    ${exe} 
    PRIVATE 
      project_warnings 
      project_sanitizers
  )
endforeach()

