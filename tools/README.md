# Tools

These command line tools can be used to compile shaders into modules and working with compiled modules.

# mslc

The `mslc` tool can be used to compile shaders into modules.

Usage: `mslc [options] -c config -o output file1 [file2...]`

In order to determine how to compile the shader, a target configuration file must be provided. This configuration file takes the form of name/value pairs.

For example:

	target = glsl-es
	version = 300
	force-disable = UniformBuffers
	force-disable = Derivatives
	remap-depth-range = yes

## Main options

* **\-h/\-\-help**: display the help message
* **\-c/\-\-config _arg_**: configuration file describing the target
* **\-i/\-\-input _arg_**: input file to compile. Multiple inputs may be provided to compile into a single module.
* **\-o/\-\-output _arg_**: output file for the compiled result
* **\-I/\-\-include _arg_**: directory to search for includes
* **\-D/\-\-define _arg_**: add a define for the preprocessor. A value may optionally be assigned with =. (i.e. `-D DEFINE=val`)
* **\-w/\-\-warn-none**: disable all warnings
* **\-W/\-\-warn-error**: treat warnings as errors
* **\-s/\-\-strip**: strip debug symbols
* **\-O/\-\-optimize**: optimize the compiled result

## Options in target configuration file

* **target = _arg_**: the target to compile for. Possible values are: spirv, glsl, glsl-es, metal-osx, metal-ios
* **version = _arg_**: the version of the target. Required for GLSL and Metal.
* **define = _arg_**: add a define for the preprocessor. A value may optionally be assigned with =. (i.e. DEFINE=val)
* **force-enable = _arg_**: force a feature to be enabled
* **force-disable = _arg_**: force a feature to be disabled
* **resources = _arg_**: a path to a file describing custom resource limits. This uses the same format as glslangValidator.
* **spirv-command = _arg_**: external command to run on the intermediate SPIR-V. The string `$input` will be replaced by the input file path, while the string `$output` will be replaced by the output file path.
* **remap-variables = _arg_**: boolean value for whether or not to remap variable ranges to improve compression of SPIR-V.
* **dummy-bindings = _arg_**: boolean value for whether or not to add dummy bindings to be changed later for SPIR-V; this will generally be done with a copy of the data.
* **adjustable-bindings = _arg_**: boolean value for whether or not to allow bindings to be adjusted in-place from the client library for SPIR-V; this also enables dummy-bindings.
* **remap-depth-range = _arg_**: boolean for whether or not to remap the depth range from \[0, 1\] to \[-1, 1\] in the  vertex shader output for GLSL targets. Defaults to false.
* **flip-vertex-y = _arg_**: boolean for whether or not to flip the vertex y coordinate for Metal targets. Defaults to true.
* **default-float-precision = _arg_**: the default precision to use for floats in GLSL targets. Possible values are: none, low, medium, high. Defaults to medium.
* **default-int-precision = _arg_**: the default precision to use for ints in in GLSL targets. Possible values are: none, low, medium, high. Defaults to high.
* **header-line = _arg_**: header line to be added verbatim for GLSL targets. This will be used for all stages.
* **header-line-vert = _arg_**: header line to be added verbatim for GLSL targets. This will be used for the vertex stage.
* **header-line-tess-ctrl = _arg_**: header line to be added verbatim for GLSL targets. This will be used for the tessellation control stage.
* **header-line-tess-eval = _arg_**: header line to be added verbatim for GLSL targets. This will be used for the tessellation evaluation stage.
* **header-line-geom = _arg_**: header line to be added verbatim for GLSL targets. This will be used for the geometry stage.
* **header-line-frag = _arg_**: header line to be added verbatim for GLSL targets. This will be used for the fragment stage.
* **header-line-comp = _arg_**: header line to be added verbatim for GLSL targets. This will be used for the compute stage.
* **extension = _arg_**: required extension to be used for GLSL targets.This will be used for all stages.
* **extension-vert = _arg_**: required extension to be used for GLSL targets. This will be used for the vertex stage.
* **extension-tess-ctrl = _arg_**: required extension to be used for GLSL targets. This will be used for the tessellation control stage.
* **extension-tess-eval = _arg_**: required extension to be used for GLSL targets. This will be used for the tessellation evaluation stage.
* **extension-geom = _arg_**: required extension to be used for GLSL targets. This will be used for the geometry stage.
* **extension-frag = _arg_**: required extension to be used for GLSL targets. This will be used for the fragment stage.
* **extension-comp = _arg_**: required extension to be used for GLSL targets. This will be used for the compute stage.
* **glsl-command-vert = _arg_**: external command to run on GLSL targets for the vertex stage. The string $input will be replaced by the input file path, while the string $output will be replaced by the output file path.
* **glsl-command-tess-ctrl = _arg_**: external command to run on GLSL targets for the tessellation control stage. The string $input will be replaced by the input file path, while the string $output will be replaced by the output file path.
* **glsl-command-tess-eval = _arg_**: external command to run on GLSL targets for the tessellation evaluation stage. The string $input will be replaced by the input file path, while the string $output will be replaced by the output file path.
* **glsl-command-geom = _arg_**: external command to run on GLSL targets for the vertex stage. The string $input will be replaced by the input file path, while the string $output will be replaced by the output file path.
* **glsl-command-frag = _arg_**: external command to run on GLSL targets for the fragment stage. The string $input will be replaced by the input file path, while the string $output will be replaced by the output file path.
* **glsl-command-comp = _arg_**: external command to run on GLSL targets for the compute stage. The string $input will be replaced by the input file path, while the string $output will be replaced by the output file path.

## Features available for force-enable and force-disable

### Types

* **Integers**: Integer types.
* **Doubles**: Double types.
* **NonSquareMatrices**: Non-square matrices, such as mat3x4.
* **Texture3D**: 3D textures.
* **TextureArray**: Texture arrays.
* **ShadowSamplers**: Shadow texture samplers.
* **MultisampledTextures**: Multisampled texture samplers.
* **IntegerTextures**: Integer texture samplers.
* **Images**: Image types.

### Storage

* **UniformBlocks**: Uniform blocks. If disabled, uniform blocks will be in the push constant section. (equivalent to individual uniforms)
* **Buffers**: Shader storage buffers.
* **Std140**: std140 block layout.
* **Std430**: std430 block layout.
* **BindingPoints**: Allows setting the binding index in the layout.
* **DescriptorSets**: Allows setting the descriptor set index in the layout.

### Pipeline stages

* **TessellationStages**: Tessellation control and evaluation stages.
* **GeometryStage**: Geometry stage.
* **ComputeStage**: Compute stage.

### Fragment shader output
* **MultipleRenderTargets**: Supports writing to more than one render target.
* **DualSourceBlending**: Supports outputting two colors to the same output.
* **DepthHints**: Allow hints to be given about the depth output.

### Other functionality

* **Derivatives**: dFdx() an dFdy() functions.
* **AdvancedDerivatives**: Coarse and fine versions of dFdx() and dFdy().
* **MemoryBarriers**: Memory barrier functions.
* **PrimitiveStreams**: Primitive streams for geometry shaders.
* **InterpolationFunctions**: Interpolation functions for centroid, sample, and offset.
* **TextureGather**: textureGather() functions.
* **TexelFetch**: texelFetch() functions.
* **TextureSize**: textureSize() functions.
* **TextureQueryLod**: texureQueryLod() functions.
* **TextureQueryLevels**: textureQueryLevels() functions.
* **TextureSamples**: textureSamples() functions.
* **BitFunctions**: Integer functions such as bitfieldInsert() and findMSB().
* **PackingFunctions**: Packing and unpacking functions such as packUnorm2x16().
* **SubpassInputs**: Subpass inputs for reading directly from framebuffers.

# mslb\-extract

Extract a compiled shader module into its components.

Usage: `mslb-extract -o output file`

The shader for each pipeline stage will be written to the output directory. The name will be based on the module name, pipelien name, and have an extension based on the stage.

* \<module\>.\<pipeline\>.vert
* \<module\>.\<pipeline\>.tesc
* \<module\>.\<pipeline\>.tese
* \<module\>.\<pipeline\>.frag
* \<module\>.\<pipeline\>.geom
* \<module\>.\<pipeline\>.comp

Unused stages will have no output file. The format of the file will depend on the target, and may either be text or binary.

Additionally, the following two files will be output:

* **\<module\>.json**: json file describing each pipeline in the module.
* **\<module\>.shared**: the shared data (only for certain targets)

## Options

* **\-h/\-\-help**: display this help message
* **\-i/\-\-input _arg_**: input shader module file to extract
* **\-o/\-\-output _arg_**: output directory to extract to. This will be created if it doesn't exist.
