#ifndef ELF_PARSER_H
#define ELF_PARSER_H

#include <cstdint>
#include <elf.h>
#include <filesystem>
#include <memory>
#include <vector>


// ============================================================================
// ELF 파서 공통 인터페이스
// ============================================================================
class ElfParser {
public:
  explicit ElfParser(const std::filesystem::path& filepath);
  ~ElfParser();

  // fstream을 다루므로 복사는 금지하고 이동만 허용
  ElfParser(ElfParser&&) noexcept;
  ElfParser& operator=(ElfParser&&) noexcept;
  ElfParser(const ElfParser&) = delete;
  ElfParser& operator=(const ElfParser&) = delete;

  // 공개 메서드
  auto extractTextSection() -> std::vector<uint8_t>;

  class ImplBase;

private:
  // 내부 구현체에 대한 전방 선언
  std::unique_ptr<ImplBase> impl_;
};


#endif // ELF_PARSER_H
