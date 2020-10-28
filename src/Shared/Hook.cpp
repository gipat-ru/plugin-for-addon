#include "Hook.h"
#include "Common.h"
#include "Patcher.h"
#include "hde/hde32.h"
#include <cassert>

using namespace SimpleHooker86;
using namespace Patcher;

static size_t GetInstrLen(const void *addr)
{
    hde32s hs;
    size_t size = hde32_disasm(addr, &hs);
    return (hs.flags & HDE_F_ERROR) ? 0 : size;
}

static size_t CalcPatchSize(const void *codeAddr)
{
    size_t size = 0;
    while (size < CallJmpSize)
    {
        size_t len = GetInstrLen((char *)codeAddr + size);
        if (len == 0)
            return 0;
        size += len;
    }
    return size;
}

class CHelpersGenerator
{
public:
    CHelpersGenerator()
        : _memorySize(0)
        , _memory(nullptr)
        , _currentAddr(nullptr)
        , _unlockAddr(nullptr)
    {
        InitMemory();
    }

    ~CHelpersGenerator()
    {
        assert(_unlockAddr == nullptr);
        FreeMemory();
    }

    void *CreateFunctionWrapper(const void *func, size_t argsSize, ECallingConvention callConv);

    void *CreateCallbackWrapper(const void *codeAddr, EOrigCodePosition codePos,
                                const void *callback, void *cbArg);

    void *CreateConverter(ECallingConvention conv);

    void *CreateTrampoline(const void *codeAddr);

    void Align()
    {
        auto &addr = _currentAddr;
        size_t alignment = sizeof(void *);
        if (addr > _memory)
            addr += WriteNops(addr, alignment - (uintptr_t)addr % alignment);
    }

    void MemLock(bool revert = false)
    {
        if (revert)
        {
            WriteFill(_unlockAddr, 0, _currentAddr - _unlockAddr);
            _currentAddr = _unlockAddr;
        }

        DWORD oldProtect;
        BOOL res = VirtualProtect(_memory, _memorySize, PAGE_EXECUTE_READ, &oldProtect);
        VERIFY(res);
        // Confirm made changes
        FlushInstructionCache(GetCurrentProcess(), _memory, _memorySize);
        _unlockAddr = nullptr;
    }

    void MemUnlock()
    {
        DWORD oldProtect;
        BOOL res = VirtualProtect(_memory, _memorySize, PAGE_READWRITE, &oldProtect);
        VERIFY(res);
        _unlockAddr = _currentAddr;
    }

private:
    size_t _memorySize;
    char *_memory;
    char *_currentAddr;
    char *_unlockAddr;

    void InitMemory()
    {
        // Should be enough for at least 128 trampolines
        _memorySize = 16384;
        // Reserve one extra page to catch out of range write
        _memory = (char *)VirtualAlloc(NULL, _memorySize + 4096, MEM_RESERVE, PAGE_EXECUTE_READ);
        VERIFY(_memory != nullptr);
        _currentAddr = _memory;

        void *mem = VirtualAlloc(_memory, _memorySize, MEM_COMMIT, PAGE_NOACCESS);
        VERIFY(mem == _memory);
    }

    void FreeMemory() { VirtualFree(_memory, 0, MEM_RELEASE); }
};
static CHelpersGenerator helpersGen;

void *CHelpersGenerator::CreateFunctionWrapper(const void *func, size_t argsSize,
                                               ECallingConvention callConv)
{
    if (argsSize > 64 || argsSize % 4 != 0)
        return nullptr;

    bool isCdecl = callConv == SimpleHooker86::CC_CDECL ||
                   callConv == SimpleHooker86::CC_THISCALL_VAR;
    bool isFastCall = callConv == SimpleHooker86::CC_FASTCALL;
    bool isThisCall = callConv == SimpleHooker86::CC_THISCALL;
    char *result = _currentAddr;
    auto &addr = _currentAddr;

    // clang-format off
    addr += WriteBytes(addr,
        0x55,                                          // push   ebp
        0x89, 0xE5,                                    // mov    ebp, esp
        0x51,                                          // push   ecx
        0x52);                                         // push   edx

    // Copy original function arguments
    for (int i = argsSize + 4; i > 4; i -= 4)
        addr += WriteBytes(addr, 0xFF, 0x75, i);       // push   dword [ebp + i]
                                                       // ; fastcall:
    addr += isFastCall ? WriteBytes(addr, 0x52) : 0;   // push   edx ; arg1 (fastcall)
    addr += (isFastCall || isThisCall)                 // ; fastcall or thiscall:
        ? WriteBytes(addr, 0x51) : 0;                  // push   ecx ; arg0 or this

    addr += WriteCall(addr, func);                     // call   callback

    // Epilog
    addr += WriteBytes(addr,
        0x8D, 0x65, 0xF8,                              // lea    esp, [ebp - 8]
        0x5A,                                          // pop    edx
        0x59,                                          // pop    ecx
        0xC9);                                         // leave
    addr += isCdecl ? WriteBytes(addr, 0xC3) :         // retn            ; cdecl
        WriteBytes(addr, 0xC2, argsSize, 0x00);        // retn   argsSize ; other
    // clang-format on

    return result;
}

void *CHelpersGenerator::CreateCallbackWrapper(const void *codeAddr, EOrigCodePosition codePos,
                                               const void *callback, void *cbArg)
{
    char *result = _currentAddr;
    auto &addr = _currentAddr;
    bool before = codePos == OCP_BEFORE;
    bool after = codePos == OCP_AFTER;
    size_t codeSize = CalcPatchSize(codeAddr);
    if (codeSize == 0)
        return NULL;
    auto defaultRetAddr = (uint32_t)((char *)codeAddr + codeSize);

    addr += before ? WritePatch(addr, codeAddr, codeSize) : 0; // ... Original bytes ...
    addr += WriteByte(addr, 0x60);                             // pushad
    addr += WriteByte(addr, 0x9C);                             // pushfd
    addr += WritePush(addr, defaultRetAddr);                   // push   retaddr
    addr += WriteBytes(addr, 0x89, 0xE5);                      // mov    ebp, esp
    addr += WritePush(addr, RN_EBP);                           // push   ebp
    addr += WritePush(addr, (uint32_t)cbArg);                  // push   cbArg
    addr += WriteCall(addr, callback);                         // call   callback
    addr += WriteBytes(addr, 0x89, 0xEC);                      // mov    esp, ebp
    addr += WritePop(addr, RN_EAX);                            // pop    eax
    addr += WriteByte(addr, 0x9D);                             // popfd
    addr += WriteByte(addr, 0x61);                             // popad
    addr += after ? WritePatch(addr, codeAddr, codeSize) : 0;  // ... Original bytes ...
    addr += WriteBytes(addr, 0xFF, 0x64, 0x24, 0xD8);          // jmp    dword [esp - 0x28]

    return result;
}

void *CHelpersGenerator::CreateConverter(ECallingConvention conv)
{
    char *result = _currentAddr;
    auto &addr = _currentAddr;
    static const uint8_t std2this[] = {
        0x59,             // pop   ecx
        0x87, 0x0C, 0x24, // xchg  [esp], ecx
    };
    static const uint8_t std2fast[] = {
        0x5A,                   // pop   edx
        0x87, 0x54, 0x24, 0x04, // xchg  edx, [esp + 4]
        0x59,                   // pop   ecx
    };

    switch (conv)
    {
    case SimpleHooker86::CC_STDCALL:
        /* Do nothing */
        break;
    case SimpleHooker86::CC_CDECL:
        return nullptr; // Not implemented yet
    case SimpleHooker86::CC_FASTCALL:
        addr += WritePatch(addr, std2fast);
        break;
    case SimpleHooker86::CC_THISCALL:
        addr += WritePatch(addr, std2this);
        break;
    case SimpleHooker86::CC_THISCALL_VAR:
        return nullptr; // Not implemented yet
    }

    return result;
}

void *CHelpersGenerator::CreateTrampoline(const void *codeAddr)
{
    size_t size = CalcPatchSize(codeAddr);
    char *result = _currentAddr;
    auto &addr = _currentAddr;
    addr += WritePatch(addr, codeAddr, size);
    addr += WriteJump(addr, (char *)codeAddr + size);
    return result;
}

bool SimpleHooker86::CreateFuncHook(void *sourceFunc, size_t argsSize, ECallingConvention callConv,
                                    const void *targetFunc, void **origFunc, Patcher::SPatch &patch)
{
    size_t patchSize = CalcPatchSize(sourceFunc);
    if (patchSize == 0)
        return false;

    void *wrap, *tramp;
    helpersGen.MemUnlock();
    helpersGen.Align();

    tramp = helpersGen.CreateConverter(callConv);
    if (!tramp)
        goto fail;

    if (helpersGen.CreateTrampoline(sourceFunc) == nullptr)
        goto fail;

    helpersGen.Align();
    wrap = helpersGen.CreateFunctionWrapper(targetFunc, argsSize, callConv);
    if (wrap == nullptr)
        goto fail;
    helpersGen.MemLock();

    patch.SetAddr(sourceFunc);
    patch.WriteJump(wrap);
    patch.WriteNops(patchSize - CallJmpSize);
    if (origFunc)
        *origFunc = tramp;
    return true;

fail:
    helpersGen.MemLock(true);
    return false;
}

bool SimpleHooker86::CreateFuncCallHook(void *callInstr, size_t argsSize,
                                        ECallingConvention callConv, const void *targetFunc,
                                        Patcher::SPatch &patch)
{
    if (GetInstrLen(callInstr) != CallJmpSize)
        return false;

    helpersGen.MemUnlock();
    helpersGen.Align();
    void *wrap = helpersGen.CreateFunctionWrapper(targetFunc, argsSize, callConv);
    bool result = wrap != nullptr;
    helpersGen.MemLock(!result);

    if (result)
        patch.WriteCall(callInstr, wrap);
    return result;
}

bool SimpleHooker86::CreateCodeHook(void *codeAddr, EOrigCodePosition codePos,
                                    CodeHookCallbackPtr callback, void *callbackArg,
                                    Patcher::SPatch &patch)
{
    size_t patchSize = CalcPatchSize(codeAddr);
    if (patchSize == 0)
        return false;

    helpersGen.MemUnlock();
    helpersGen.Align();
    void *wrapper = helpersGen.CreateCallbackWrapper(codeAddr, codePos, callback, callbackArg);
    if (!wrapper)
        goto fail;
    helpersGen.MemLock();

    patch.SetAddr(codeAddr);
    patch.WriteJump(wrapper);
    patch.WriteNops(patchSize - CallJmpSize);
    return true;

fail:
    helpersGen.MemLock(true);
    return false;
}
