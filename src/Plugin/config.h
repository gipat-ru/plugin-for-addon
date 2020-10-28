#pragma once

#include <string>
#include <windows.h>

struct SPluginConfig
{
    int EnablePK;
    std::string StartZoneName;

    SPluginConfig()
        : EnablePK(0)
        , StartZoneName("")
    {
    }

    void Load(const std::string &configPath)
    {
        //
        // Config file example:
        // [Example]
        // PK = 1
        // StartZoneName = gz12k
        //

        EnablePK = GetPrivateProfileIntA("Example", "PK", EnablePK, configPath.c_str());

        char buf[1024];
        GetPrivateProfileStringA("Example", "StartZone", StartZoneName.c_str(), buf, sizeof(buf),
                                 configPath.c_str());
        StartZoneName = buf;
    }
};

extern SPluginConfig PluginConfig;
