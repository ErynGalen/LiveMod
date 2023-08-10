#ifndef H_ASM_GLOBAL_LLVM_H
#define H_ASM_GLOBAL_LLVM_H

#include <iostream>
#include <llvm/ADT/Triple.h>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCRegisterInfo.h>
#include <llvm/MC/MCSubtargetInfo.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/ManagedStatic.h>
#include <memory>

class GlobalLLVM {
  public:
    GlobalLLVM() = default;
    ~GlobalLLVM() { llvm::llvm_shutdown(); };

    bool init();

#define CHECK_INIT                                                                                                     \
    if (!m_initialized) {                                                                                              \
        std::cout << "[LLVM] Not initialized!" << std::endl;                                                           \
    }

    const llvm::Target *target() { CHECK_INIT return m_pTarget; }
    llvm::MCSubtargetInfo *subTargetInfo() { CHECK_INIT return m_pSubTargetInfo.get(); }
    llvm::MCRegisterInfo *registerInfo() { CHECK_INIT return m_pRegisterInfo.get(); }
    llvm::MCAsmInfo *asmInfo() { CHECK_INIT return m_pAsmInfo.get(); }
    llvm::MCContext *context() { CHECK_INIT return m_pContext.get(); }
#undef CHECK_INIT
    GlobalLLVM(const GlobalLLVM &) = delete;
    GlobalLLVM &operator=(const GlobalLLVM &) = delete;

  private:
    bool m_initialized = false;
    llvm::Triple m_targetTriple;
    const llvm::Target *m_pTarget;
    std::unique_ptr<llvm::MCSubtargetInfo> m_pSubTargetInfo;
    std::unique_ptr<llvm::MCRegisterInfo> m_pRegisterInfo;
    std::unique_ptr<llvm::MCAsmInfo> m_pAsmInfo;
    std::unique_ptr<llvm::MCContext> m_pContext;
};

extern GlobalLLVM g_LLVM;

#endif
