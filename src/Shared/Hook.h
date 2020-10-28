#pragma once

#include "Shared/Patcher.h"
#include <stdint.h>

#if !defined(_M_IX86) && !defined(__i386__)
#error "Hook.h supports only x86 systems"
#endif

namespace SimpleHooker86 {

enum ECallingConvention
{
    CC_STDCALL,
    CC_CDECL,
    CC_FASTCALL,
    CC_THISCALL,

    // A version of thiscall with variable arguments.
    // The same as cdecl but with *this as the 1st stack arg.
    CC_THISCALL_VAR,
};

/*
 * Creates a hook for the specified source function.
 *
 * This function creates a trampoline which transfers control
 * to the specified target function with corresponding calling convention
 * of the source function and returns a pointer to the trampoline.
 */
bool CreateFuncHook(void *sourceFunc, size_t argsSize, ECallingConvention callConv,
                    const void *targetFunc, void **origFunc, Patcher::SPatch &patch);

/*
 * Create a hook for the specified function call to the source function.
 *
 * This function creates a trampoline which transfers control
 * to the specified target function with corresponding calling convention
 * of the source function and returns a pointer to the trampoline.
 */
bool CreateFuncCallHook(void *callInstr, size_t argsSize, ECallingConvention callConv,
                        const void *targetFunc, Patcher::SPatch &patch);

#if defined(_MSC_VER) && defined(_MSC_EXTENSIONS)
#pragma warning(push)
#pragma warning(disable : 4201) // nonstandard extension used : nameless struct/union
#endif

#define _HOOK_DECLARE_REGISTER_SHORT(name32, name16) \
    union                                            \
    {                                                \
        uint32_t name32;                             \
        uint16_t name16;                             \
    }

#define _HOOK_DECLARE_REGISTER_FULL(name32, name16, name8l, name8h) \
    union                                                           \
    {                                                               \
        uint32_t name32;                                            \
        uint16_t name16;                                            \
        struct                                                      \
        {                                                           \
            uint8_t name8l;                                         \
            uint8_t name8h;                                         \
        };                                                          \
    }

#pragma pack(push, 1)
struct SCodeHookCtx
{
    void *RetAddr;
    uint32_t EFlags;
    _HOOK_DECLARE_REGISTER_SHORT(Edi, Di);
    _HOOK_DECLARE_REGISTER_SHORT(Esi, Si);
    _HOOK_DECLARE_REGISTER_SHORT(Ebp, Bp);
    _HOOK_DECLARE_REGISTER_SHORT(Esp, Sp);
    _HOOK_DECLARE_REGISTER_FULL(Ebx, Bx, Bl, Bh);
    _HOOK_DECLARE_REGISTER_FULL(Edx, Dx, Dl, Dh);
    _HOOK_DECLARE_REGISTER_FULL(Ecx, Cx, Cl, Ch);
    _HOOK_DECLARE_REGISTER_FULL(Eax, Ax, Al, Ah);
};
#pragma pack(pop)

#if defined(_MSC_VER) && defined(_MSC_EXTENSIONS)
#pragma warning(pop)
#endif

enum EOrigCodePosition
{
    OCP_IGNORE, // Don't copy orig code
    OCP_BEFORE, // Copy orig code before calling the callback
    OCP_AFTER,  // Copy orig code after calling the callback
};

typedef void(__stdcall *CodeHookCallbackPtr)(void *callbackArg, SCodeHookCtx *context);

bool CreateCodeHook(void *codeAddr, EOrigCodePosition codePos, CodeHookCallbackPtr callback,
                    void *callbackArg, Patcher::SPatch &patch);

} // namespace SimpleHooker86
