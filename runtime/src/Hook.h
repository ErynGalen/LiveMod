#ifndef H_HOOK_H
#define H_HOOK_H

#include <cstddef>
#include <stdio.h> // temporary

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

    void deb() {
        printf("- %d, %d\n", (int)m_hookSize, (int)m_trampolineSize);
        for (unsigned char *byte_addr = (unsigned char *)m_pTrampoline;
             byte_addr < (unsigned char *)m_pTrampoline + m_trampolineSize; byte_addr++) {
            printf("0x%x ", *byte_addr);
        }
        printf("\n");
    }

    void *original() { return m_pTrampoline; }

    bool set_source(void *source) {
        if (m_isHooked) {
            return false;
        }
        m_pSource = source;
        return true;
    }
    bool set_destination(void *destination) {
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
