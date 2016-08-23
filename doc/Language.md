# Language

The core language of MSL is [GLSL 450](https://www.opengl.org/registry/doc/GLSLangSpec.4.50.pdf) with the [Vulkan language extensions](https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt).

# Preprocessing

The shader file is first run through a C99 preprocessor. The following \#defines are present:

## Target defines

* `SPIRV_VERSION`: (SPIR-V target only) The version SPIR-V version number.
* `GLSL_VERSION`: (Desktop GLSL target only) The GLSL version. (e.g. 120, 450)
* `GLSLES_VERSION`: (GLSL ES target only) The GLSL version. (e.g. 100, 300)
* `METAL_OSX_VERSION`: (OS X Metal target only) The metal version times 10. (e.g. 10)
* `METAL_IOS_VERSION`: (iOS Metal target only) The metal version times 10. (e.g. 10, 11)

## Feature defines

### Types

* `HAS_INTEGERS`: Set to 1 if integer types are available, 0 if not.
* `HAS_DOUBLES`: Set to 1 if double types are available, 0 if not.
* `HAS_NON_SQUARE_MATRICES`: Set to 1 if non-square matrices, such as mat3x4, are available 0 if not.
* `HAS_TEXTURE3D`: Set to 1 if 3D textures are available, 0 if not.
* `HAS_TEXTURE_ARRAY`: Set to 1 if texture arrays are available, 0 if not.
* `HAS_SHADOW_SAMPLERS`: Set to 1 if shadow texture samplers are available, 0 if not.
* `HAS_MULTISAMPLED_TEXTURES`: Set to 1 if multisampled texture samplersare available, 0 if not.
* `HAS_INTEGER_TEXTURES`: Set to 1 if integer texture samplers are available, 0 if not.
* `HAS_IMAGES`: Set to 1 if image types are available, 0 if not.

### Storage

* `HAS_UNIFORM_BUFFERS`: Set to 1 if uniform buffers are available, 0 if not. If disabled, uniform buffers will be converted to push constants.(equivalent to individual uniforms)
* `HAS_BUFFERS`: Set to 1 if shader storage buffers are available, 0 if not.
* `HAS_STD140`: Set to 1 if the std140 block layout is available, 0 if not.
* `HAS_STD430`: Set to 1 if the std430 block layout is available, 0 if not.
* `HAS_BINDING_POINTS`: Set to 1 if binding point layout is available, 0 if not.
* `HAS_DESCRIPTOR_SETS`: Set to 1 if descriptor set layout is available, 0 if not.

### Pipeline stages

* `HAS_TESSELLATION_STAGES`: Set to 1 if tessellation control and evaluation stages are available, 0 if not.
* `HAS_GEOMETRY_STAGE`: Set to 1 if the geometry stage is available, 0 if not.
* `HAS_COMPUTE_STAGE`: Set to 1 if the compute stage is available, 0 if not.

### Fragment shader output

* `HAS_MULTIPLE_RENDER_TARGETS`: Set to 1 if writing to more than one render target is available, 0 if not.
* `HAS_DUAL_SOURCE_BLENDING`: Set to 1 if outputting two colors to the same output, 0 if not.
* `HAS_DEPTH_HINTS`: Set to 1 if hints can be given about the depth output, 0 if not.

### Other functionality

* `HAS_DERIVATIVES`: Set to 1 if the dFdx() an dFdy() functions are available, 0 if not.
* `HAS_ADVANCED_DERIVATIVES`: Set to 1 if the coarse and fine versions of dFdx() and dFdy() are available, 0 if not.
* `HAS_MEMORY_BARRIERS`: Set to 1 if memory barrier functions are available, 0 if not.
* `HAS_PRIMITIVE_STREAMS`: Set to 1 if primitive streams are available for geometry shaders, 0 if not.
* `HAS_INTERPOLATION_FUNCTIONS`: Set to 1 if interpolation functions for centroid, sample, and offset are available, 0 if not.
* `HAS_TEXTURE_GATHER`: Set to 1 if textureGather() functions are available, 0 if not.
* `HAS_TEXEL_FETCH`: Set to 1 if texelFetch() functions are available, 0 if not.
* `HAS_TEXTURE_SIZE`: Set to 1 if textureSize() functions are available, 0 if not.
* `HAS_TEXTURE_QUERY_LOD`: Set to 1 if texureQueryLod() functions are available, 0 if not.
* `HAS_TEXTURE_QUERY_LEVELS`: Set to 1 if textureQueryLevels() functions are available, 0 if not.
* `HAS_TEXTURE_SAMPLES`: Set to 1 if textureSamples() functions are available, 0 if not.
* `HAS_BIT_FUNCTIONS`: Set to 1 if integer functions such as bitfieldInsert() and findMSB() are available, 0 if not.
* `HAS_PACKING_FUNCTIONS`: Set to 1 if packing and unpacking functions such as packUnorm2x16() are available, 0 if not.
* `HAS_SUBPASS_INPUTS`: Set to 1 if subpass inputs for reading directly from framebuffers are available, 0 if not.

# Filtering

MSL compiles the same source file for all pipeline stages, so filters may be used to ensure that certain elements are only included for specific stages. Filters are surrounded by `[[]]` and can be before any top-level element in the source file, such as:

* Global variables
* Uniforms and uniform blocks
* Inputs and output variables and interface blocks
* Functions

The following pipeline stages can be used:

* vertex
* tessellation_control
* tessellation_evaluation
* geometry
* fragment
* compute

Multiple pipelines can be used, separated with commas.

For example:

	uniform sampler2D tex;

	uniform Transform
	{
		mat4 transform;
	} block;

	[[vertex]] in vec3 position;
	[[vertex]] in vec4 color;

	[[vertex]] out VertexOut
	{
		vec4 color;
	} outputs;

	[[fragment]] in FragmentIn
	{
		vec4 color;
	} inputs;

	[[fragment]] out vec4 color;

	[[vertex, fragment]]
	int getValue()
	{
		return 10;
	}

	[[vertex]]
	void vertShader()
	{
		gl_Position = block.transform*vec4(position, 1.0);
		outputs.color = color*getValue();
	}

	[[fragment]]
	void fragShader()
	{
		vec4 texResult = texture(tex, vec2(0.5, 0.5));
		color = inputs.color*texResult/getValue();
	}

# Pipelines

Pipelines can be declared within the shader with the `pipeline` keyword. Each pipeline has a name, and declares the entry point functions for the following stages:

* vertex
* tessellation_control
* tessellation_evaluation
* geometry
* fragment
* compute

Stages that aren't used can be omitted.

For example, the following pipeline declaration can be used for the previous example:

	pipeline Test
	{
		vertex = vertShader;
		fragment = fragShader;
	}
