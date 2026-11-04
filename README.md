# RISC-V (RV32I) Disassembler & ELF Parser

RISC-V (RV32I) 명령어 세트를 디코딩하고 ELF32 포맷의 실행 파일을 분석하는 고성능 C++ 기반 디스어셈블러 엔진 프로젝트입니다.

## 프로젝트 핵심 가치 및 제작 동기
* **학습 및 검증:** David A. Patterson의 *Computer Organization and Design (RISC-V Edition)*을 스터디하며 이론으로 접한 프로세서의 파이프라인 구조를 소프트웨어 아키텍처로 직접 구현해 검증하고자 시작했습니다.
* **하드웨어 아키텍처의 재현:** 실제 CPU의 **IF (Instruction Fetch)** 단계와 **ID (Instruction Decode)** 단계의 매커니즘을 C++의 다형성(Polymorphism)과 팩토리 패턴(Factory Pattern)을 통해 논리적 구조로 재현했습니다.

---

## 주요 기능 (Key Features)

### 1. 하드웨어 스테이지 지향적 다형성 설계
* **ID Stage 추적:** 최상위 추상 클래스 `Instruction`을 중심으로, 인스트럭션 비트의 Opcode(7-bit)를 분석해 1차 분기하고, `funct3` 및 `funct7` 필드를 통해 2차 분기하여 구체적인 인스트럭션 객체를 동적 생성합니다.
* **안정적인 자원 관리:** 대규모 인스트럭션 스트림 디코딩 시 메모리 누수를 원천 차단하기 위해 `std::unique_ptr` 스마트 포인터 기반의 팩토리 아키텍처를 구축했습니다.

### 2. 가변 길이 명령어 및 확장형 디코딩
* **C Extension (16-bit) 대응:** 입력을 1바이트 단위 버퍼로 관리하며, `(bits & 0x3) != 0x3` 비트 마스킹 연산을 통해 16비트 압축 명령어와 32비트 표준 명령어를 실시간 판별합니다.
* **동적 PC 제어:** 명령어 포맷 판별 결과에 따라 가중치(2 또는 4)를 주어 디스어셈블러의 PC(Program Counter) 주소를 동적으로 정밀 제어합니다.

### 3. 정밀한 비트 필드 조립 및 부호 확장 (Sign Extension)
* **Immediate 조각 복원:** B-Type, S-Type 등 명령어 포맷별로 흩어져 있는 비트 필드 조각들을 비트 쉬프트 및 OR 연산(`|`)을 통해 정밀하게 재조립합니다.
* **비트 마스킹 부호 확장:** 비트 연산 후 `static_cast<int32_t>(raw_imm << 19) >> 19` 구조를 활용하여 32비트 환경에 맞는 완벽한 부호 확장 연산을 수행합니다.

---

## 개발 환경 및 활용 도구
* **Language:** C++20 (`std::format`, 모던 속성 및 키워드 활용)
* **OS / Environment:** Fedora Linux (WSL), CMake
* **Toolchain / 검증:** Clang-Tidy, ASan (AddressSanitizer), LLVM Project (`llvm-objcopy`, `llvm-objdump`)
* **Test Framework:** Google Test (`googletest`) 통합 교차 검증

---

## 검증 프로세스 및 테스트 결과

본 프로젝트는 작성한 소스코드가 실제 RISC-V 크로스 컴파일러의 바이너리 결과물과 일치하는지 `googletest` 기반의 통합 테스트를 통해 교차 검증합니다.

### 1. 테스트 시나리오 구현 (`test.c`)
배열의 합을 구하는 루프 및 함수 호출 구조가 포함된 C 코드를 기반으로 테스트 베드를 구축했습니다.

```c
int sum(int arr[], int n) {
    int total = 0;
    int *iter = arr;
    int *end = arr + n;
    for (; iter != end; ++iter) { total += *iter; }
    return total;
}
```

### 2. 크로스 컴파일 및 순수 바이너리 추출
```bash
# 1. riscv32 타겟으로 오브젝트 파일 빌드
clang --config ~/etc/clang/riscv32.cfg -O1 -c test.c -o test.out

# 2. 메타데이터를 제외한 순수 .text 기계어 섹션만 바이너리로 추출
llvm-objcopy -0 binary --only-section=.text test.out test.bin
```

### 3. 교차 검증 결과 대조
자체 디스어셈블러 출력 결과가 LLVM 표준 도구인 `llvm-objdump` 공식 결과와 인스트럭션, 레지스터 번호, Immediate 값까지 완벽히 일치함을 검증했습니다.

| 본 프로젝트 `disasm-rv32i_test` 결과 | 오피셜 `llvm-objdump` 결과 |
| :--- | :--- |
| `0x00000000 0x00058063 beq x11, x0, 0x00000000` | `0: 00058063 beq x11, x0, 0x0 <sum>` |
| `0x0000000c 0x00259593 slli x11, x11, 2` | `c: 00259593 slli x11, x11, 0x2` |
| `0x00000010 0x00062683 lw x13, 0(x12)` | `10: 00062683 lw x13, 0x0(x12)` |

---

## 한계점 및 향후 로드맵 (Current Limitations & Roadmap)

* **현재 한계점:** 메타데이터가 제거된 순수 `.text` 바이너리 스트림(`.bin`)만 수용 가능하며, 완전한 실행 파일(`.out`) 직접 입력 시 ELF 파일 구조(헤더, 메타데이터 부문)까지 명령어 코드로 오인해 디코딩을 시도하다 `unknown`이 발생하는 한계가 있습니다.
* **향후 계획 (Next Goal):** 
  * 실행 파일(`.out`)을 직접 파싱할 수 있는 **ELF32 Parser** 모듈 구현 중.
  * ELF 헤더(`Elf32_Ehdr`) 분석을 통한 프로그램 실제 시작 주소(`e_entry`) 동적 추출 기능 연동 예정.
  * 섹션 헤더 테이블(`Elf32_Shdr`) 자동 추적을 통한 `.text` 실행 영역 자동 메모리 매핑 아키텍처 설계 중.
