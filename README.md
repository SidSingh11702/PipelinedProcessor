# Pipelined Processor Simulator

A C++ implementation that simulates a 5-stage pipelined processor with stall handling and performance statistics tracking.

## Authors
- Akshat Meena
- Siddharth Singh

## Overview

This project implements a simulator for a pipelined processor architecture with 5 stages (Instruction Fetch, Instruction Decode, Execution, Memory Access, and Write Back). The simulator handles data dependencies, control hazards, and monitors performance metrics including instruction counts, cycles per instruction, and stall statistics.

## Features

- **5-Stage Pipeline Implementation**:
  - Instruction Fetch (IF)
  - Instruction Decode (ID)
  - Execution (EX)
  - Memory Access (MEM)
  - Write Back (WB)

- **Hazard Handling**:
  - Data Hazards (RAW - Read After Write)
  - Control Hazards (Branch instructions)
  - Stall Implementation

- **Instruction Set**:
  - Arithmetic instructions (add, subtract, multiply, increment)
  - Logical instructions (AND, OR, NOT, XOR)
  - Data instructions (load, store)
  - Control instructions (jump, branch if zero)
  - Halt instruction

- **Memory Components**:
  - 16 Registers (Register File)
  - 256-entry Instruction Cache
  - 256-entry Data Cache

- **Performance Tracking**:
  - Total instructions executed
  - Instructions per class (Arithmetic, Logical, Data, Control, Halt)
  - Cycles per instruction (CPI)
  - Total stalls
  - Data stalls vs. Control stalls

## Input Files

The simulator requires three input files placed in an `input` directory:

1. `RF.txt` - Initial values for the Register File
2. `ICache.txt` - Instruction cache contents
3. `DCache.txt` - Data cache contents

## Output Files

The simulator generates two output files in an `output` directory:

1. `DCache.txt` - Final state of the Data Cache
2. `Output.txt` - Performance statistics including:
   - Total instruction count
   - Instruction type breakdown
   - Cycles per instruction
   - Stall statistics

## Instruction Format

Instructions are stored in hexadecimal format with each instruction consisting of 4 characters:
- First character: operation code (opcode)
- Next three characters: operands

## Usage

1. Prepare the input files in the `input` directory
2. Compile the source code:
   ```
   g++ PipelinedProcessor.cpp -o processor
   ```
3. Run the program:
   ```
   ./processor
   ```
4. View results in the `output` directory

## Implementation Details

- The processor handles RAW (Read After Write) data hazards by stalling the pipeline
- Control hazards from branch instructions are handled by stalling and flushing the pipeline
- A halt instruction properly terminates execution and reports statistics
- The simulation includes comprehensive tracking of various instruction types and performance metrics
