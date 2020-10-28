#include "patches.h"
#include "config.h"
#include "Shared/Common.h"
#include "Shared/Patcher.h"
#include <windows.h>

using namespace Common;

// Implement your patches here
// TODO: More complex example with using Hook library

static bool PKPatch(Patcher::SPatch &patch)
{
    if (!PluginConfig.EnablePK)
        return false;

    patch.WriteByte((void *)0x554144, 0xEB);
    return true;
}

static bool StartZonePatch(Patcher::SPatch &patch)
{
    if (PluginConfig.StartZoneName.empty())
        return false;

    // Copy config parameter to protect from config changes
    static const auto zone = PluginConfig.StartZoneName;

    patch.WritePush((void *)0x4A5D17, (uint32_t)zone.c_str());
    return true;
}

typedef bool (*PatchFunction)(Patcher::SPatch &patch);
static const PatchFunction Patches[] = {
    // Put your patch functions here:
    PKPatch,
    StartZonePatch,
};

// Below is a code to apply patches

static DWORD _OldProtections[2];
static char *_BaseAddr = (char *)0x400000;

static void GameMemLock()
{
    DWORD tmp;
    // Lock .text
    if (!VirtualProtect(_BaseAddr + 0x1000, 0x33A000, _OldProtections[0], &tmp))
        ErrorMsgBox(1, "Cannot lock game .text section. VirtualProtect failed: 0x%X",
                    (unsigned)GetLastError());
    FlushInstructionCache(GetCurrentProcess(), _BaseAddr + 0x1000, 0x33A000);
    // Lock .rdata
    if (!VirtualProtect(_BaseAddr + 0x33B000, 0x48000, _OldProtections[1], &tmp))
        ErrorMsgBox(1, "Cannot lock game .rdata section. VirtualProtect failed: 0x%X",
                    (unsigned)GetLastError());
}

static void GameMemUnlock()
{
    // Unlock .text
    if (!VirtualProtect(_BaseAddr + 0x1000, 0x33A000, PAGE_EXECUTE_READWRITE, &_OldProtections[0]))
        ErrorMsgBox(1, "Cannot unlock game .text section. VirtualProtect failed: 0x%X",
                    (unsigned)GetLastError());
    // Unlock .rdata
    if (!VirtualProtect(_BaseAddr + 0x33B000, 0x48000, PAGE_READWRITE, &_OldProtections[1]))
        ErrorMsgBox(1, "Cannot unlock game .rdata section. VirtualProtect failed: 0x%X",
                    (unsigned)GetLastError());
}

void ApplyPatches()
{
    GameMemUnlock();
    for (size_t i = 0; i < _countof(Patches); i++)
    {
        Patcher::SPatch patch;
        if (!Patches[i](patch))
            continue; // Skip disabled patch

        for (auto it = patch.Chunks.cbegin(); it != patch.Chunks.cend(); ++it)
            memcpy(it->Addr, &it->Data[0], it->Data.size());
    }
    GameMemLock();
}
