#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <cstdint>
#include <elf.h>
#include <filesystem>
#include <memory>
#include <vector>


// ============================================================================
// 1. ELF 파서 공통 인터페이스
// ============================================================================
class ElfParser {
public:
  virtual ~ElfParser() = default;
  virtual auto extractTextSection() -> std::vector<uint8_t> = 0;
  static auto createElfParser(const std::filesystem::path& filepath) 
    -> std::unique_ptr<ElfParser>;
};


#endif // ELF_PARSER_H
