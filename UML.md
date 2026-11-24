# RV32I Disassembler — UML

기준 커밋: [`4785541`](https://github.com/Jun19204/rv32i-disassembler-engine/tree/4785541a001ba22de242ab9bbd864ef8151fadbd)

실제 구현을 바탕으로 전체 의존 관계와 명령어 상속 관계를 나누어 표시한다. 반복되는 생성자와 일부 구현 세부 사항은 생략한다.

## 1. 전체 구조

`Main`은 클래스가 아닌 `main()` 자유 함수를 나타낸다. 메서드의 `const` 한정자는 가독성을 위해 생략했다.

```mermaid
classDiagram
direction TB

class Main {
  <<function>>
  main(argc, argv) int
}
class Elf32Parser {
  -std::vector~std::byte~ buffer_
  +Elf32Parser(filepath)
  +getSection(sec_name) std::optional~BinaryRegion~
  +getSegment(seg_name) std::optional~BinaryRegion~
  +getTextRegion() std::optional~BinaryRegion~
  -readAt~T~(offset) T
  -slice(offset, count) std::span~const std::byte~
  -validateHeader() void
}
class BinaryRegion {
  <<struct>>
  +std::span~const std::byte~ data_
  +const uint32_t begin_vma_
}
class ToDecodeInstrInfo {
  <<struct>>
  +const uint32_t pc_
  +const uint32_t bits_
  +const size_t length_
}
class Rv32iDecoder {
  +decode(uint32_t bits) std::unique_ptr~Instruction~$
  -decodeRType(bits) std::unique_ptr~Instruction~$
  -decodeITypeLoad(bits) std::unique_ptr~Instruction~$
  -decodeITypeArithLogicShift(bits) std::unique_ptr~Instruction~$
  -decodeITypeJump(bits) std::unique_ptr~Instruction~$
  -decodeSType(bits) std::unique_ptr~Instruction~$
  -decodeBType(bits) std::unique_ptr~Instruction~$
  -decodeJType(bits) std::unique_ptr~Instruction~$
  -decodeUType(bits) std::unique_ptr~Instruction~$
}
class Instruction {
  <<abstract>>
  #const uint32_t bits_
  #const std::string mnemonic_
  +disassemble(uint32_t pc) std::string*
}

Main ..> Elf32Parser : 생성 및 영역 추출
Main ..> BinaryRegion : 바이트 뷰와 시작 VMA 사용
Main ..> ToDecodeInstrInfo : vector로 보관
Main ..> Rv32iDecoder : decode 호출
Main ..> Instruction : unique_ptr로 소유 및 호출
Elf32Parser ..> BinaryRegion : optional로 반환
Rv32iDecoder ..> Instruction : 파생 객체 생성 및 반환
```

### 소유권과 수명

- `Elf32Parser::buffer_`가 ELF 파일의 바이트 데이터를 소유한다.
- `BinaryRegion::data_`는 해당 버퍼의 일부를 참조하는 비소유 `std::span`이다. `BinaryRegion`을 복사해도 바이트 데이터는 복사되지 않는다. 뷰를 사용하는 동안 원본 버퍼가 유효해야 한다.
- `Rv32iDecoder`는 생성한 명령어를 멤버로 보관하지 않는다. `std::unique_ptr<Instruction>`을 반환하여 호출자에게 소유권을 전달한다.
- `main()`의 `decoded_instr`가 반환된 객체를 소유한다. `Instruction`의 가상 소멸자를 통해 파생 객체가 올바르게 소멸한다.

## 2. 명령어 상속 구조

모든 구체 명령어 클래스는 `rv32i_decoder.cpp`의 익명 네임스페이스에 정의되어 있다. 외부 공개 인터페이스는 `Instruction`과 `Rv32iDecoder`이다.

```mermaid
classDiagram
direction LR

class Instruction {
  <<abstract>>
  #const uint32_t bits_
  #const std::string mnemonic_
  +Instruction(bits, mne)
  +~Instruction()
  +disassemble(uint32_t pc) std::string*
}
class RType {
  <<final>>
  -uint32_t rd_
  -uint32_t rs1_
  -uint32_t rs2_
  +disassemble(uint32_t pc) std::string
}
class ITypeArithLogic {
  #uint32_t rd_
  #uint32_t rs1_
  #int32_t imm_
  +disassemble(uint32_t pc) std::string
}
class ITypeLoadJalr {
  <<final>>
  +disassemble(uint32_t pc) std::string
}
class ITypeShift {
  <<final>>
  -uint32_t rd_
  -uint32_t rs1_
  -uint32_t shamt_
  +disassemble(uint32_t pc) std::string
}
class SType {
  <<final>>
  -uint32_t rs1_
  -uint32_t rs2_
  -int32_t imm_
  +disassemble(uint32_t pc) std::string
}
class BType {
  <<final>>
  -uint32_t rs1_
  -uint32_t rs2_
  -int32_t imm_
  +disassemble(uint32_t pc) std::string
}
class JType {
  <<final>>
  -uint32_t rd_
  -int32_t imm_
  +disassemble(uint32_t pc) std::string
}
class UType {
  <<final>>
  -uint32_t rd_
  -uint32_t imm_
  +disassemble(uint32_t pc) std::string
}
class Unknown {
  <<final>>
  +disassemble(uint32_t pc) std::string
}

Instruction <|-- RType
Instruction <|-- ITypeArithLogic
ITypeArithLogic <|-- ITypeLoadJalr
Instruction <|-- ITypeShift
Instruction <|-- SType
Instruction <|-- BType
Instruction <|-- JType
Instruction <|-- UType
Instruction <|-- Unknown
```

`ITypeLoadJalr`는 `ITypeArithLogic`의 생성자를 상속하여 `rd_`, `rs1_`, `imm_` 추출을 재사용하고, 출력 형식만 재정의한다. `ITypeArithLogic`은 자체적으로도 생성되는 구체 클래스이다.

`Instruction::disassemble()`은 순수 가상 함수이며 모든 구체 클래스에서 재정의한다. `Instruction`의 소멸자는 가상 함수이다. `Unknown`은 생성자를 `Instruction`으로부터 상속하고 `"unknown"`을 출력한다.

## 3. 파일별 구성

| 파일 | 주요 구성 요소 |
| --- | --- |
| `elf32_parser.h` | `BinaryRegion`, `Elf32Parser`, 시스템 `elf.h`가 없는 경우의 ELF 구조체 정의 |
| `elf32_parser.cpp` | 파일 로딩, 헤더 검증, 섹션 및 세그먼트 탐색 |
| `rv32i_decoder.h` | `Instruction`, `Rv32iDecoder` |
| `rv32i_decoder.cpp` | 구체 명령어 클래스, `Opcode` 열거형, 디코딩 팩토리 구현 |
| `disasm-rv32i.cpp` | `main()`, `ToDecodeInstrInfo`, `UnsignedInteger`, `readLE()` |

## 표기

| 표기 | 의미 |
| --- | --- |
| `+` / `-` / `#` | `public` / `private` / `protected` |
| 빈 삼각형 실선 화살표 | 상속 관계. 화살표가 부모 클래스를 가리킴 |
| 점선 화살표 | 사용 또는 생성에 따른 의존 관계 |
| `abstract` | 추상 클래스 |
| `final` | 추가 상속을 금지한 클래스 |
| 메서드 끝의 `*` | 추상 메서드. Mermaid에서 기울임꼴로 표시 |
| 메서드 끝의 `$` | 정적 메서드. Mermaid에서 밑줄로 표시 |

ELF 헤더 구조체와 표준 라이브러리 타입의 내부 구조, `Opcode` 및 자유 보조 함수는 클래스 다이어그램에서 생략했다.
