# NCDC Assembly Module Project - RISC-V Assembler

## Course
**NCDC Cohort 02/2025 - Design Verification (DV)**  
NUST Chip Design Centre

## Module
RISC-V Assembly Module

## Project Description
Implementation of a **RISC-V Assembler** that translates RISC-V assembly instructions into machine code (binary/hex).

The RISC-V ISA (Instruction Set Architecture) is an open standard instruction set architecture based on established RISC principles.

### Supported Instruction Types
- **R-Type**: Register-register operations (ADD, SUB, AND, OR, XOR, SLL, SRL, SRA)
- **I-Type**: Immediate operations (ADDI, LW, JALR)
- **S-Type**: Store instructions (SW, SH, SB)
- **B-Type**: Branch instructions (BEQ, BNE, BLT, BGE)
- **U-Type**: Upper immediate (LUI, AUIPC)
- **J-Type**: Jump instructions (JAL)

### Features
- Parses RISC-V assembly text files
- Converts instructions to 32-bit binary machine code
- Supports labels and pseudo-instructions
- Generates output in hex format
- Two-pass assembler for label resolution

## Files
- `ASsembler_project.zip` - Source code for the RISC-V assembler
- `NCDC_Assembler_project_report.pdf` - Project report with design details and test results

## Submission Details
- **Submitted:** Tuesday, 26 August 2025
- **Module:** RISC-V Assembly
- **Grade:** 4.64 / 100.00
