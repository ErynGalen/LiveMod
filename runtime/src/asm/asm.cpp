#include "asm.h"
#include "GlobalLLVM.h"
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/raw_ostream.h>

using namespace llvm;

size_t instructionLength(uint8_t *addr, size_t maxLength) {
    std::vector<uint64_t> lengths;

    std::unique_ptr<MCDisassembler> disAsm(
        g_LLVM.target()->createMCDisassembler(*g_LLVM.subTargetInfo(), *g_LLVM.context()));
    if (!disAsm) {
        std::cout << "Couldn't create MCDisassembler" << std::endl;
        return 0;
    }

    ArrayRef<uint8_t> bytes(addr, maxLength);
    uint64_t instructionLength = 0;
    MCInst instruction;
    MCDisassembler::DecodeStatus status =
        disAsm->getInstruction(instruction, instructionLength, bytes, (uint64_t)addr, nulls());
    switch (status) {
    case MCDisassembler::DecodeStatus::Fail:
        std::cout << "Unknown instruction: ";
        instruction.print(outs());
        std::cout << std::endl;
        return 0;

    case MCDisassembler::DecodeStatus::SoftFail:
        std::cout << "Weird instruction: ";
        instruction.print(outs());
        std::cout << std::endl;
        [[fallthrough]];

    case MCDisassembler::DecodeStatus::Success:

        return instructionLength;
    }
    return instructionLength; // shouldn't happen
}
