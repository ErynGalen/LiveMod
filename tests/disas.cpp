//! Just a reference for how to set up a disassembler with LLVM,
//! how to print instructions, get their size, etc.

#include <cstdint>
#include <iostream>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCInstPrinter.h>
#include <llvm/MC/MCInstrInfo.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSchedule.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/ARMTargetParser.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetParser.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <vector>

using namespace llvm;

std::vector<MCInst> instructions(const MCDisassembler &disAsm,
                                 std::vector<uint8_t> bytes,
                                 raw_ostream &output) {
    ArrayRef<uint8_t> data(bytes.data(), bytes.size());

    std::vector<MCInst> instrs;
    uint64_t instrSize = 0;
    for (uint64_t pos = 0; pos < bytes.size(); pos += instrSize) {
        MCInst instr;
        MCDisassembler::DecodeStatus status = disAsm.getInstruction(
            instr, instrSize, data.slice(pos), pos, output);
        switch (status) {
        case MCDisassembler::DecodeStatus::Fail:
            std::cout << "Invalid instruction" << std::endl;
            break;
        case MCDisassembler::DecodeStatus::SoftFail:
            std::cout << "Weird instruction" << std::endl;

        case MCDisassembler::DecodeStatus::Success:
            std::cout << "Decoded instruction, size = " << instrSize
                      << std::endl;
            instr.print(output);
            output << '\n';
            instrs.push_back(instr);
        }
    }

    return instrs;
}

int main(int argc, char **argv) {
    InitLLVM X(argc, argv);

    InitializeAllTargetInfos();
    InitializeAllTargetMCs();
    InitializeAllDisassemblers();

    std::string tripleName = sys::getProcessTriple();
    Triple triple(Triple::normalize(tripleName));
    std::cout << "Using triple " << triple.getTriple() << std::endl;

    std::string targetError;
    const Target *target =
        TargetRegistry::lookupTarget(tripleName, targetError);
    if (!target) {
        std::cout << "Can't create target: " << targetError << std::endl;
    }
    std::unique_ptr<MCSubtargetInfo> subTargetInfo(
        target->createMCSubtargetInfo(tripleName, "", ""));
    if (!subTargetInfo) {
        std::cout << "Can't create sub target info" << std::endl;
        return 1;
    }

    MCRegisterInfo *registerInfo = target->createMCRegInfo(tripleName);
    MCTargetOptions options{};
    MCAsmInfo *asmInfo =
        target->createMCAsmInfo(*registerInfo, tripleName, options);
    MCInstrInfo *instrInfo = target->createMCInstrInfo();

    MCContext context(triple, asmInfo, registerInfo, &*subTargetInfo);

    std::vector<uint8_t> buffer = {0x90, 0x50, 0x58, 0x64, 0x48, 0x8b,
                                   0x04, 0x25, 0x28, 0x00, 0x00, 0x00};

    std::unique_ptr<MCDisassembler> disAsm(
        target->createMCDisassembler(*subTargetInfo, context));
    if (!disAsm) {
        std::cout << "Can't create disassembler" << std::endl;
    }

    auto instrs = instructions(*disAsm, buffer, outs());
    MCInstPrinter *printer = target->createMCInstPrinter(
        triple, 0, *asmInfo, *instrInfo, *registerInfo);
    for (MCInst &i : instrs) {
        printer->printInst(&i, 0, "", *subTargetInfo, outs());
        outs() << '\n';
    }
    std::cout.flush();

    return 0;
}
