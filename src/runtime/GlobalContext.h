#ifndef H_CONTEXT_H
#define H_CONTEXT_H

class GlobalContext {
  public:
    bool m_isNative = false;
};

extern GlobalContext g_Context;

class MakeNativeGuard {
    bool m_wasNative;

  public:
    MakeNativeGuard() {
        m_wasNative = g_Context.m_isNative;
        if (!m_wasNative) {
            g_Context.m_isNative = true;
        }
    }
    ~MakeNativeGuard() { g_Context.m_isNative = m_wasNative; }
};

#endif
