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

bool InstructionInfo::atAddr(uint8_t *addr, size_t maxLength) {
    GlobalLLVM *llvm = getGlobalLLVMContext();

    std::unique_ptr<MCDisassembler> disAsm(
        llvm->target()->createMCDisassembler(*llvm->subTargetInfo(), *llvm->context()));
    if (!disAsm) {
        std::cout << "[Asm] Couldn't create MCDisassembler" << std::endl;
        return false;
    }

    ArrayRef<uint8_t> bytes(addr, maxLength);
    uint64_t instructionLength = 0;
    MCInst instruction;
    MCDisassembler::DecodeStatus status =
        disAsm->getInstruction(instruction, instructionLength, bytes, (uint64_t)addr, nulls());
    switch (status) {
    case MCDisassembler::DecodeStatus::Fail:
        std::cout << "[Asm] Unknown instruction: ";
        instruction.print(outs());
        std::cout << std::endl;
        return false;

    case MCDisassembler::DecodeStatus::SoftFail:
        std::cout << "[Asm] Warning: Weird instruction: ";
        instruction.print(outs());
        std::cout << std::endl;
        [[fallthrough]];

    case MCDisassembler::DecodeStatus::Success:
        m_length = instructionLength;
        // check for program counter usages
        unsigned int programCounterRegister = llvm->registerInfo()->getProgramCounter().id();
        for (unsigned int op_n = 0; op_n < instruction.getNumOperands(); op_n++) {
            auto operand = instruction.getOperand(op_n);
            if (operand.isReg() && operand.getReg() == programCounterRegister) {
                m_usesProgramCounter = true;
                break;
            }
        }
        return true;
    }

    exit(-1); // shouldn't happen
}
