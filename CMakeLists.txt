cmake_minimum_required(VERSION 3.23)

project(
  DISASSEMBLER-RV32I 
  VERSION 1.0.0
  LANGUAGES CXX
)


if(MSVC)
  add_compile_options(/utf-8)
  if(POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    # CXX_COMPILER_ID 단독 검사로 수정
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT 
      "$<IF:$<CXX_COMPILER_ID:MSVC>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>"
    )
  endif()
endif()


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


add_library(RV32I_Decoder_lib STATIC rv32i_decoder.cpp)
add_library(ELF32_Parser_lib STATIC elf32_parser.cpp)
target_include_directories(RV32I_Decoder_lib PUBLIC "${PROJECT_SOURCE_DIR}")
target_include_directories(ELF32_Parser_lib PUBLIC "${PROJECT_SOURCE_DIR}")
target_compile_features(RV32I_Decoder_lib PUBLIC cxx_std_23)
target_compile_features(ELF32_Parser_lib PUBLIC cxx_std_23)


add_executable(disasm-rv32i disasm-rv32i.cpp)
target_link_libraries(disasm-rv32i PRIVATE
  RV32I_Decoder_lib
  ELF32_Parser_lib
)
target_compile_features(disasm-rv32i PRIVATE cxx_std_23)

