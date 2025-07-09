# Stellarnet Legacy
Python bindings for legacy StellarNet spectrometers.

## Components
+ `stellarnet-legacy`: Python API library.
+ `stellarnet_api`: 64-bit C++ message broker that relays messages bewteen `stellarnet-legacy` and `stellarnet_api_server`.
+ `stellarnet_api_server`: 32-bit C++ server that handles commands from `sterallnet_api` and delegates them to the native Stellarnet DLL.

## `stellarnet_api` and `stellarnet_api_server` development
Open the `stellarnet_api` project in Visual Studio. It includes the `stellarnet_api_server` project. 
`stellarnet_api_server` must be compiled to 32-bit architecture to correclty load the StellarNet DLL.
Post build events for both `stellarnet_api` and `stellarnet_api_server` copy the final dll and executable to the correct location in the Python package.