# OBIELF – OBINexus Executable and Linkable Format

Welcome to the official OBIELF repository – a forward-looking ELF format specification and toolchain under active research by the OBINexus R&D Division. OBIELF is designed to produce **portable, minimal, and architecture-aware** binaries that can be assembled using NASM and executed reliably across compliant Linux systems.

---

## 🚀 Project Vision
**OBIELF** stands for **OBINexus Executable and Linkable Format**. It’s a clean, minimal ELF variant that can:

- Run without glibc or dynamic linkers
- Embed policy metadata and trust signals
- Support NASM (`nasm -f obielf64`) and future DSL compilers
- Map AST structures and bytecode directly into ELF sections

Our mission is to define an ELF-compatible, policy-aware execution model that speaks the language of modern secure compilation.

---

## 📁 Repository Structure
```txt
obielf/
├── specs/                # OBIELF format specifications
│   ├── obielf.md         # Core layout and architecture
│   └── sections.md       # Custom section definitions (.obi.*)
├── tools/                # ELF analysis and validation tools
│   ├── obielf-check      # Validator for OBIELF compliance
│   └── generate-uuid     # UUID tagging utility for ELF builds
├── examples/             # Sample NASM programs using OBIELF
│   ├── hello.asm         # Hello World using OBIELF
│   └── raw_syscall.asm   # Minimal ELF with no glibc
├── nasm-plugin/          # Prototype backend for NASM (future)
│   └── obielf64.patch    # Experimental OBIELF format patch
├── README.md             # This file
└── LICENSE               # Licensing information (MIT or OBINexus Custom)
```

---

## 🔧 Features
- **System-V Compatible ELF64** base
- `.obi.*` sections for:
  - Policy metadata
  - UUID and semantic fingerprints
  - AXC transformation history
- Raw syscall support (no libc required)
- NASM-first, future support for AST-aware compilers
- Clean, bounded memory layout

---

## 🔍 Sample Output
```nasm
section .obi.axc
    db 'AXC optimized: true', 0
section .obi.uuid
    db 'build:bf19d2e0-71aa-421c-9b35', 0
```

```bash
$ readelf -S hello.obielf
[22] .obi.axc          PROGBITS        ...
[23] .obi.uuid         PROGBITS        ...
```

---

## 🛠 Getting Started
### 1. Assemble with NASM
```bash
nasm -f elf64 hello.asm -o hello.o
ld -o hello hello.o
```

### 2. Inspect with readelf
```bash
readelf -a hello
```

### 3. Validate OBIELF Format (optional)
```bash
./tools/obielf-check hello
```

---

## 🧪 In Development
- [ ] `nasm -f obielf64` official format
- [ ] ELF32 + ELF64 support
- [ ] Cross-platform build suite (ARM64, RISC-V)
- [ ] Compiler backend for AST → OBIELF
- [ ] Integration with `GOSI` for secure signing

---

## 📚 Resources
- [Universal ELF Tutorial](../docs/Universal-ELF.md)
- [AST-Aware Architecture](https://obinexus.org/docs/ast-aware)
- [https://wiki.osdev.org/ELF](https://wiki.osdev.org/ELF)

---

## 📄 License
OBIELF is open for use and experimentation under the [MIT License](LICENSE), with contributions governed by OBINexus Computing.

---

## 🧠 Author & Maintainers
**Nnamdi Michael Okpala** – Lead Developer & Research Engineer, OBINexus Computing  
Contact: [https://obinexus.org](https://obinexus.org)

---

> "Not just executable. Knowable, traceable, trusted."  
> — OBIELF Design Philosophy