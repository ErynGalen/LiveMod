#ifndef H_HOOK_H
#define H_HOOK_H

#include <cstddef>

class Hook {
    void *m_pSource      = nullptr;
    void *m_pDestination = nullptr;
    void *m_pTrampoline  = nullptr;

    size_t m_hookSize       = 0; // size needed at the beginning of the hooked function
    size_t m_trampolineSize = 0;

    bool m_isHooked = false;

  public:
    Hook() = default;
    ~Hook() {
        if (m_isHooked) {
            unhook();
        }
    }
    bool hook();
    bool unhook();

    void *original() { return m_pTrampoline; }

    bool setSource(void *source) {
        if (m_isHooked) {
            return false;
        }
        m_pSource = source;
        return true;
    }
    bool setDestination(void *destination) {
        if (m_isHooked) {
            return false;
        }
        m_pDestination = destination;
        return true;
    }

    Hook(const Hook &)            = delete;
    Hook &operator=(const Hook &) = delete;
};

/// retrieve `orig_func` from the specified hook, whose arguments have the types specified in `...`
#define ORIGINAL(hook, ret_type, func, ...)                                                                            \
    ret_type (*orig_##func)(__VA_ARGS__) = (ret_type(*)(__VA_ARGS__))hook.original()

#endif
