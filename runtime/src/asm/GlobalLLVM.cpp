#include "GlobalLLVM.h"

#include <iostream>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCDwarf.h>
#include <llvm/MC/MCInst.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/MCTargetOptions.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetSelect.h>
#include <memory>

using namespace llvm;

bool GlobalLLVM::init() {
    if (m_initialized) {
        return true;
    }
    InitializeAllTargetInfos();
    InitializeAllTargetMCs();
    InitializeAllDisassemblers();

    m_targetTriple = Triple(Triple::normalize(sys::getProcessTriple()));
    std::string error;
    m_pTarget = TargetRegistry::lookupTarget(m_targetTriple.getTriple(), error);
    if (!m_pTarget) {
        std::cout << "[LLVM] Couldn't initialize target: " << error << std::endl;
        return false;
    }

    m_pSubTargetInfo = std::unique_ptr<MCSubtargetInfo>(
        m_pTarget->createMCSubtargetInfo(m_targetTriple.getTriple(), sys::getHostCPUName(), ""));
    if (!m_pSubTargetInfo) {
        std::cout << "[LLVM] Couldn't initialize SubTargetInfo" << std::endl;
        return false;
    }

    m_pRegisterInfo = std::unique_ptr<MCRegisterInfo>(m_pTarget->createMCRegInfo(m_targetTriple.getTriple()));
    if (!m_pRegisterInfo) {
        std::cout << "[LLVM] Couldn't initialize RegisterInfo" << std::endl;
        return false;
    }

    MCTargetOptions targetOptions;
    m_pAsmInfo = std::unique_ptr<MCAsmInfo>(
        m_pTarget->createMCAsmInfo(*m_pRegisterInfo, m_targetTriple.getTriple(), targetOptions));
    if (!m_pAsmInfo) {
        std::cout << "[LLVM] Couldn't initialize AsmInfo" << std::endl;
        return false;
    }

    m_pContext =
        std::make_unique<MCContext>(m_targetTriple, m_pAsmInfo.get(), m_pRegisterInfo.get(), m_pSubTargetInfo.get());
    if (!m_pContext) {
        std::cout << "[LLVM] Couldn't initialize Context" << std::endl;
        return false;
    }

    m_initialized = true;
    return true;
}

GlobalLLVM g_LLVM;
// use the highest priority to ensure that LLVM is available to other constructors
__attribute__((constructor(101))) void init_LLVM() { g_LLVM.init(); }
