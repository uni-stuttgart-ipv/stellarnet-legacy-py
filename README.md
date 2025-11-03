# Stellarnet Legacy
Python bindings for legacy StellarNet spectrometers.

**All programs must be run as admin.**

## Components
+ `stellarnet-legacy`: Python API library.
+ `stellarnet_api`: 64-bit C++ message broker that relays messages bewteen `stellarnet-legacy` and `stellarnet_api_server`.
+ `stellarnet_api_server`: 32-bit C++ server that handles commands from `sterallnet_api` and delegates them to the native Stellarnet DLL.

## `stellarnet_api` and `stellarnet_api_server` development
+ The `stellarnet_api` project in Visual Studio. It includes the `stellarnet_api_server` project. 
+ `stellarnet_api_server` must be compiled to 32-bit architecture to correclty load the StellarNet DLL.
+ Post build events for both `stellarnet_api` and `stellarnet_api_server` copy the final dll and executable to the correct location in the Python package.

### Get started
+ Ensure Visual Studio (not VS Code) is installed with the `Desktop development with C++` workload installed. You may have to install additional packages, such as the `Windows 11 SDK`.
+ Ensure the Stellarnet drivers are installed. You may have to replace the default drivers with updated ones.
0. Open `stellarnet_api/stellarnet_api.sln` in Visual Studio (not VS Code) using **Run as administrator**.
1. Open the solution properties (right click on `Solution stellarnet_api` in the `Solution explorer` pane) and set `Common properties > Configure Startup Projects` to `Single startup project` with `stellarnet_api_server` as the main project. The rest of the solution's settings should be set as needed.
2. Start debugging the program by poressing `F5`. This should open a terminal
3. Test the program is running correctly by submitting the command `stellarnet_path`. The response should be something like `C:\Program Files\StellarNet\SpectraWiz`.
4. Submit the `open` command. You must run this before running any other command. If an error occurs, ensure you are running Visual Studio as an administrator.
5. You should not be able to run any command you want.

## Logging
+ Logging requires [`spdlog`](https://github.com/gabime/spdlog) to be installed.

## Troubleshooting
### (Python) When running `stellarnet_legacy.open()` my program crashes.
This is likely because you are not running the program as an admin.

### (C++) My program crashes at `SWDinit()` with `KernelBase.dll exception code 0x0EEDFADE`
This is likely because you are not running the program as an admin.
