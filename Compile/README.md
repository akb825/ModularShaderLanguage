# Compile

This library can be used to compile shader files into modules to be consumed by clients.

In order to compile a shader, you must first have an instance of a `msl::Target` subclass. The targets currently available are:

* `msl::TargetSpirV`: compiles to SPIR-V to be used with Vulkan.
* `msl::TargetGlsl`: compiles to GLSL for either desktop OpenGL and OpenGL ES. You can target nearly every GLSL version, allowing you to use the same shader source for many different OpenGL targets. The \#defines for the various capabilities can be used to enable and disable features for the different targets.
* `msl::TargetMetal`: compiles to Apple Metal. This will use Apple's compiler to compile and link the components of the compiled module into a single library.

A `msl::Target` instance compiles one or more shader files into a `msl::CompiledResult` instance, which can be used to save to the final compiled shader module. The target will first compile the shader to SPIR-V, which can then be cross-compiled to other targets.

When compiling through a `msl::Target`, the following must be done:

* Add include paths through `msl::Target::addIncludePath()`
* Add additional \#defines through `msl::Target::addDefine()`
* Additional target-specific options may be set on the Target.
* For each shader to compile, call `msl::Target::compile()`
* Once each shader has been compiled, call `msl::Target::finish()`
* Save the result by calling `msl::CompiledResult::save()`

Warnings and errors from the compiler will be sent to the `msl::Output` class passed as parameters to the functions, which can be used to print to the console or anywhere else.

The following may also be optionally set on `msl::Target`:

* Specific features may be overridden with `msl::Target::overrideFeature()`. This can be used to force features to be disabled (e.g. work around bugs on drivers) or enabled. (e.g. an extension is available)
* Variables can be remapped with `msl::Target::setRemapVariables()`. This can make SPIR-V output compress better.
* Optimizations can be applied with `msl::Target::setOptimize()`. These are simple optimizations such as dead code elimination and load-store reductions.
* Debug symbols can be stripped with `msl::Target::setStripDebug()`. This will reduce the size for SPIR-V and remove local variable names when cross-compiling to other languages such as GLSL.
* A resource configuration file can be set with `msl::Target::setResourcesFileName()`. This is the same format as used by [glslangValidator](https://www.khronos.org/opengles/sdk/tools/Reference-Compiler/).
* An external tool can be used to process the SPIR-V with `msl::Target::setSpirVToolCommand()`. (e.g. a tool to apply more aggressive optimizations) The string `$input` will be replaced with the input file and `$output` wil be replaced with the output file.

The following may optionally be set on `msl::TargetGlsl`:

* The depth range may be remapped from \[0, 1\] (used by Vulkan) to \[-1, 1\] (used by OpenGL) in the vertex stage output with `msl::TargetGlsl::setRemapDepthRange()`.
* The default float precision may be set with `msl::TargetGlsl::setDefaultFloatPrecision()` and the default int precision may be set with `msl::TargetGlsl::setDefaultIntPrecision()`.
* Add a line to be inserted verbatim at the start of the file with `msl::TargetGlsl::addHeaderLine()`. This can be set for all stages or for specific stages.
* Add a required extension with `msl::TargetGlsl::addRequiredExtension()`. This can be set for all stages or for specific stages.
* Set an external tool to process the GLSL source can be set with `msl::TargetGlsl::setGlslToolCommand()`. (e.g. [GLSL Optimizer](https://github.com/aras-p/glsl-optimizer)) This is set per stage.

The following may optionally be set on `msl::TargetMetal`:

* The depth range may be remapped from \[0, 1\] (used by Vulkan) to \[-1, 1\] (used by Metal) in the vertex stage output with `msl::TargetMetal::setRemapDepthRange()`.
* The vertex y coordinates may be inverted with `msl::TargetMetal::setFlipVertexY()`. This is enabled by default.
* The fragment y coordinates may be inverted with `msl::TargetMetal::setFlipFragmentY()`. This is enabled by default.
