  Example of plugin for addon.dll
-----------------------------------

This is an example of a plugin for addon.dll

## How to build

### Prerequisites

- Visual Studio 2010 or newer
- CMake (optional)

### Method 1:

1. Open plugin.sln with Visual Studio
2. Press Build -> Build Solution

### Method 2 (using build.bat)

1. Open Visual Studio Command Prompt in repository directory
2. Use the following command: `build.bat debug` (or `build.bat release`)

### Method 3 (using CMake directly):

1. Open Command Prompt in repository directory
2. Use the following commands:

    ```
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

## How to enable it

Addon.dll 0.8.0 or newer is required to use plugins.

If so, modify config.reg file of addon in which you're going to use your plugin
with the following lines:

```ini
[DEV]
Plugin=plugin.dll
```

Specify the name of your plugin file name instead of `plugin.dll` if you want.
