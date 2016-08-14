# Client

This library provides a C and C++ implementation for loading compiled shader modules in client applications.

The C implementation can be found in MSL/Client/ModuleC.h, while the C++ implementation can be found in MSL/Client/ModuleCpp.h.

In either case, the shader module can be loaded from a stream, data buffer, or file. A single allocation is used to store the data for the module and metadata, which can be made with a custom allocator. See the documentation in the header file for the language you wish to use for more info.
