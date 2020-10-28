#include "patches.h"
#include "config.h"
#include "VersionInfo.h"
#include "Shared/Common.h"
#include <windows.h>

#if !defined(_M_IX86) && !defined(__i386__)
#error "Plugin supports only x86 systems"
#endif

// Calling convention used for plugin. Must be cdecl
#define PLUGIN_DECL __cdecl

int PLUGIN_DECL EntryBeforePatch(const char *modPath)
{
    // Some code executed before addon.dll patches
    // Patch the game here if you want addon to check collision (recommended)
    ApplyPatches();
    return 0;
}

int PLUGIN_DECL EntryAfterPatch(const char *modPath)
{
    // Some code executed after addon.dll patches
    // Patch the game here if you want to care about collisions with addon yourself
    return 0;
}

bool PLUGIN_DECL AddonMustReloadConfig()
{
    // Return true if you want addon to reload it's config (not implemented yet)
    return false;
}

const char *PLUGIN_DECL GetPluginVersion()
{
    // Return string version of plugin (is not used by addon yet)
    return PRODUCT_VERSION_STRING;
}

const char *PLUGIN_DECL GetPluginName()
{
    // Return name of plugin
    return PRODUCT_NAME;
}

SPluginConfig PluginConfig;

BOOL APIENTRY DllMain(HINSTANCE hInstanceDLL, DWORD fdwReason, LPVOID)
{
    switch (fdwReason)
    {
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hInstanceDLL);

        // Load config
        auto pluginDir = Common::GetDirectoryName(Common::GetCurrentModulePath());
        Common::AddTrailingSlash(pluginDir);
        PluginConfig.Load(Common::WideToAnsi(pluginDir) + "plugin.ini");

        break;
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
