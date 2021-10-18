# Language

The core language of MSL is [GLSL 450](https://www.opengl.org/registry/doc/GLSLangSpec.4.50.pdf) with the [Vulkan language extensions](https://www.khronos.org/registry/vulkan/specs/misc/GL_KHR_vulkan_glsl.txt), with the following main differences:

* Improved preprocessor.
* Filtering based on pipelines.
* Changes to how uniforms are managed.
* Declarations of full pipelines within a shader.
* Declarations of render states and sampler states.

# Preprocessing

The shader file is first run through a C99 preprocessor. The following \#defines are present:

## Target defines

* `SPIRV_VERSION`: (SPIR-V target only) The SPIR-V version number. The 100s digit is the major version while the 10s and 1s digits are the minor version. For example, version 1.4 would be 104.
* `GLSL_VERSION`: (Desktop GLSL target only) The GLSL version. (e.g. 120, 450)
* `GLSLES_VERSION`: (GLSL ES target only) The GLSL version. (e.g. 100, 300)
* `METAL_VERSION`: (OS X or iOS Metal target only) The Metal version number. The 100s digit is the major version while the 10s and 1s digits are the minor version. For example, version 2.1 would be 201.
* `METAL_OSX_VERSION`: (OS X Metal target only) The Metal version number set only for OS X.
* `METAL_IOS_VERSION`: (iOS Metal target only) The Metal version number set only for iOS.

## Feature defines

### Types

* `HAS_INTEGERS`: Set to 1 if integer types are available, 0 if not.
* `HAS_DOUBLES`: Set to 1 if double types are available, 0 if not.
* `HAS_NON_SQUARE_MATRICES`: Set to 1 if non-square matrices, such as mat3x4, are available 0 if not.
* `HAS_TEXTURE3D`: Set to 1 if 3D textures are available, 0 if not.
* `HAS_TEXTURE_ARRAYS`: Set to 1 if texture arrays are available, 0 if not.
* `HAS_SHADOW_SAMPLERS`: Set to 1 if shadow texture samplers are available, 0 if not.
* `HAS_MULTISAMPLED_TEXTURES`: Set to 1 if multisampled texture samplersare available, 0 if not.
* `HAS_INTEGER_TEXTURES`: Set to 1 if integer texture samplers are available, 0 if not.
* `HAS_IMAGES`: Set to 1 if image types are available, 0 if not.

### Storage

* `HAS_UNIFORM_BUFFERS`: Set to 1 if uniform buffers are available, 0 if not. If disabled, uniform buffers will be converted to push constants.(equivalent to individual uniforms)
* `HAS_BUFFERS`: Set to 1 if shader storage buffers are available, 0 if not.
* `HAS_STD140`: Set to 1 if the `std140` block layout is available, 0 if not.
* `HAS_STD430`: Set to 1 if the `std430` block layout is available, 0 if not.
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

* `HAS_DERIVATIVES`: Set to 1 if the `dFdx()` an `dFdy()` functions are available, 0 if not.
* `HAS_ADVANCED_DERIVATIVES`: Set to 1 if the coarse and fine versions of `dFdx()` and `dFdy()` are available, 0 if not.
* `HAS_MEMORY_BARRIERS`: Set to 1 if memory barrier functions are available, 0 if not.
* `HAS_PRIMITIVE_STREAMS`: Set to 1 if primitive streams are available for geometry shaders, 0 if not.
* `HAS_INTERPOLATION_FUNCTIONS`: Set to 1 if interpolation functions for centroid, sample, and offset are available, 0 if not.
* `HAS_TEXTURE_GATHER`: Set to 1 if `textureGather()` functions are available, 0 if not.
* `HAS_TEXEL_FETCH`: Set to 1 if `texelFetch()` functions are available, 0 if not.
* `HAS_TEXTURE_SIZE`: Set to 1 if `textureSize()` functions are available, 0 if not.
* `HAS_TEXTURE_QUERY_LOD`: Set to 1 if `texureQueryLod()` functions are available, 0 if not.
* `HAS_TEXTURE_QUERY_LEVELS`: Set to 1 if `textureQueryLevels()` functions are available, 0 if not.
* `HAS_TEXTURE_SAMPLES`: Set to 1 if `textureSamples()` functions are available, 0 if not.
* `HAS_BIT_FUNCTIONS`: Set to 1 if integer functions such as `bitfieldInsert()` and `findMSB()` are available, 0 if not.
* `HAS_PACKING_FUNCTIONS`: Set to 1 if packing and unpacking functions such as `packUnorm2x16()` are available, 0 if not.
* `HAS_SUBPASS_INPUTS`: Set to 1 if subpass inputs for reading directly from framebuffers are available, 0 if not.
* `HAS_CLIP_DISTANCE`: Set to 1 if the `gl_ClipDistance` array is supported, 0 if not. Note that even if the shader language supports clip distances, the target itself may not, so additional runtime checks may be required.
* `HAS_CULL_DISTANCE`: Set to 1 if the `gl_CullDistance` array is supported, 0 if not. Note that even if the shader language supports cull distances, the target itself may not, so additional runtime checks may be required.
* `HAS_EARLY_FRAGMENT_TESTS`: Set to 1 if early fragment tests may be explicitly enabled with the `early_fragment_tests` render state.

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

	varying(vertex, fragment)
	{
		vec4 vertColor;
	}

	[[fragment]] out vec4 color;

	[[vertex, fragment]]
	int getValue()
	{
		return 10;
	}

	[[vertex]]
	void vertShader()
	{
		gl_Position = INSTANCE(block).transform*vec4(position, 1.0);
		vertColor = color*getValue();
	}

	[[fragment]]
	void fragShader()
	{
		vec4 texResult = texture(tex, vec2(0.5, 0.5));
		color = vertColor*texResult/getValue();
	}

# Inputs/outputs

## Varying blocks

Variables may be placed in a varying block to handle outputs from one stage to outputs to another stage, allowing them to be declared once for both stages rather than having two declarations for each.

The syntax of this is `varying(output, input) {...}`, where `output` is the stage to output the values from and `input` is the stage to input the values to. For example, the following block:

	varying(vertex, fragment)
	{
		vec2 texCoord;
		vec4 color;
		vec3 normal;
	}

is equivalent to the following code without a varying block:

	[[vertex]] out vec2 texCoord;
	[[vertex]] out vec4 color;
	[[vertex]] out vec3 normal;
	[[fragment]] in vec2 texCoord;
	[[fragment]] in vec4 color;
	[[fragment]] in vec3 normal;

### Tessellation control and geometry shader inputs

Inputs for tessellation control and geometry shader stages that are declared in varying blocks are automatically converted to arrays. This is **not** done for interface blocks declared within the varying block. This is because it's ambiguous if you want the members or the block itself (assuming it's named) to be an array. In this case the block will remain unchanged, which will then have a compile error when compiling the GLSL.

	varying(vertex, tessellation_control)
	{
		vec4 position; // Converted to array.
		Block // NOT converted to an array: will result in a compile error.
		{
			vec2 texCoord;
			vec4 color;
			vec3 normal;
		};
	}

## Input/output limitations

When structs are used for inputs and outputs, that struct may only be used for a single output/input pair. During linking the location information is embedded in the struct itself, which can only be used once. Additionally, you may not have nested structs (i.e. a struct member variable) for an input or output.

When linking, inputs and outputs are matched based on their undecorated names. In other words, it will look for members of any interface blocks as well as member variables. As a result, if multiple interface blocks are used, they may not share member names.

## Input/output variables for older versions of GLSL

Input and output locations are automatically calculated to link the stages together. However, older versions of GLSL don't suppprt explicit locations. In order to ensure that shaders can link in older versions of GLSL, one of these two should be done:

* Declare each input/output variable individually rather than in interface blocks.
* Make sure the struct or interface block names match between the input and output blocks. (as is done in the above example) The varying variables will be decorated by the struct or interface block name, so if they don't match OpenGL won't be able to link the inputs and examples. In the example above, the `VertexOut` struct has a member named `color`, so the varying will be named `VertexOut_color`.

# Uniforms

Uniforms blocks will be automatically removed when compiling for targets that don't support them. The members will then be treated the same as uniforms outside of uniform blocks. Buffer blocks are not removed automatically since they provide different sets of features.

Uniforms that are outside of a block (either originally or due to the removal of blocks) and don't use an opaque type (such as `sampler2D`) will be found under an implicit block called `Uniforms` with an instance name of `uniforms`.

In order to use a uniform in a block both when uniform blocks are enabled and disabled, the block should have an instance name and the `INSTANCE()` macro may be used when accessing it. If uniform blocks are disabled, the instance name will be replaced with `uniforms`. Even though instance names need to be used to support targets without uniform block support, since they are put into the same block in the end the members cannot share the same name.

For example:

	uniform MyBlock
	{
		vec4 value;
	} myBlock;

	uniform float otherValue;

	void main()
	{
		...
		vec4 val = INSTANCE(myBlock).value*uniforms.otherValue;
		...
	}

During reflection, `otherValue` will be listed as `Uniforms.otherValue`. Similarly, when uniform blocks are disabled, `MyBlock.value` will become `Uniforms.value`.

> **Note:** All of the uniforms within a compilation unit will be included for all of the compiled pipelines. If you have a lot of unused uniforms this can be inefficient, especially in targets such as Vulkan which put all of the free uniforms into a single buffer. This can be mitigated by separating groups of uniforms into separate \#include files and making sure that each compiled file only includes code that is used in the pipelines it declares, and also only contains pipelines that use mostly the same sets of uniforms.

## Reasons for this implementation

Vulkan GLSL does not support non-opaque uniforms outside of a block, and and instead uniforms not part of any other block are placed in a single push\_constant block. The tools provided to cross-compile SPIR-V transform push\_constant members into free uniforms, but there are two undesirable effects:

1. All uniforms would have to be declared in a single uniform block.
2. Uniform blocks cannot be trivially converted into free uniforms.

To make things more complicated, push\_constant uniform blocks must have instance names.

To work around these issues in a reasonable manner, MSL will take all free uniforms and uniforms removed from blocks and place them implicitly into a push\_constant block of the name Uniforms with the instance name of uniforms before compiling to SPIR-V.

In order to handle cases such as uniforms that use structs, some code re-ordering is performed. After parsing, the order of code passed to the compiler will be:

1. Default precision declarations.
2. Struct definitions.
3. Non-opaque free uniforms.
4. Uniform blocks.
5. Everything else.

The ordering within each block will be consistent with the original source, so cases such as structs containing other structs will still work.

## Sampler states

Sampler states may be declared for samplers. For example:

	uniform sampler2D tex;
	sampler_state tex
	{
		address_mode_u = repeat;
		address_mode_v = clamp_to_edge;
		min_filter = linear;
		mag_filter = linear;
		mip_filter = anisotropic;
	};

The following sampler states are allowed to be set:

* `min_filter`: set to the minification filter to use. Possible values are:
	* `nearest`
	* `linear`
* `mag_filter`: set to the magnification filter to use. Possible values are the same as `min_filter`.
* `mip_filter`: set to the mip filter to use. Possible values are:
	* `nearest`
	* `linear`
	* `anisotropic`
* `address_mode_u`: set to the how to treat the texture coordinates at the edge of the range \[0, 1\] for the U (or S) texture coordinate. Possible values are:
	* `repeat`
	* `mirrored_repeat`
	* `clamp_to_edge`
	* `clamp_to_border`
	* `mirror_once`
* `address_mode_v`: set to the how to treat the texture coordinates at the edge of the range \[0, 1\] for the V (or T) texture coordinate. Possible values are the same as `address_mode_u`.
* `address_mode_w`: set to the how to treat the texture coordinates at the edge of the range \[0, 1\] for the W (or R) texture coordinate. Possible values are the same as `address_mode_u`.
* `mip_lod_bias`: set to a floating point value for the bias to apply when calculating the mip-mapping LOD.
* `max_anisotropy`: set to a floating point value for the maximum anisotropy factor to apply.
* `min_lod`: set to a floating point value for the minimum mip level to use.
* `max_lod`: set to a floating point value for the maximum mip level to use.
* `border_color`: set to the color to use when the address mode is set to `clamp_to_border`. This can only be set to specific colors. The possible values are:
	* `transparent_black`: all color channels and alpha are 0.
	* `transparent_int_zero`: all color channels are set to the integer value 0.
	* `opaque_black`: color channels are set to 0, alpha is set to 0.
	* `opaque_int_zero`: color channels are set to 0, alpha is set to the int value 1.
	* `opaque_white`: color channels and alpha are set to 1.
	* `opaque_int_zero`: color channels are and alpha are set to the int value 1.

# Pipelines

Pipelines can be declared within the shader with the `pipeline` keyword. Each pipeline has a name, and declares the entry point functions for the following stages:

* vertex
* tessellation\_control
* tessellation\_evaluation
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

## Render states

Render states may be set on the pipeline. For example, you may enable alpha blending with the following pipeline declaration:

	pipeline Test
	{
		vertex = vertShader;
		fragment = fragShader;

		blend_enable = true;
		src_color_blend_factor = src_alpha;
		dst_color_blend_factor = one_minus_src_alpha;
		src_alpha_blend_factor = one;
		dst_alpha_blend_factor = zero;
	}

The following render states are available to set:

### Rasterization states

* `depth_clamp_enable`: set to `true` to clamp the depth values in range rather than clipping.
* `rasterizer_discard_enable`: set to `true` to discard all samplers in the rasterizer.
* `polygon_mode`: set how to rasterize polygons. Possible values are:
	* `fill`
	* `line`
	* `point`
* `cull_mode`: set whether to cull front and back faces. Possible values are:
	* `none`
	* `front`
	* `back`
	* `front_and_back`
* `front_face`: set which winding to use to determine the front face. Possible values are:
	* `counter_clockwise`
	* `clockwise`
* `depth_bias_enable`: set to true to apply depth bias to the depth result.
* `depth_bias_constant_factor`: set to a floating point value to add to the final depth value.
* `depth_bias_clamp`: set the minimum (if negative) or maximum (if positive) value to clamp the depth bias to.
* `depth_bias_slope_factor`: set to a floating point value to add to the depth result based on the polygon slope.
* `line_width`: set to a floating point value for the number of pixels wide to draw line primitives.
* `compare_op`: set the compare operation to apply for shadow samplers. Possible values are:
	* `never`
	* `less`
	* `equal`
	* `less_or_equal`
	* `greater`
	* `not_equal`
	* `greater_or_equal`
	* `always`

### Multisample states

* `sample_shading_enable`: set to `true` to run the shader for multiple samples.
* `min_sample_shading`: set to a floating point value for a hint of how many samples to use.
* `sample_mask`: set to an integer bitmask for which samples to run the shader on.
* `alpha_to_coverage_enable`: set to `true` to use the alpha value to determine how many samples to use.
* `alpha_to_one_enable`: set to `true` to force the alpha value to one.

### Depth-stencil states

* `depth_test_enable`: set to `true` to enable the depth test.
* `depth_write_enable`: set to `true` to write the depth value to the depth buffer.
* `depth_compare_op`: set the compare operation to apply for the depth test. Possible values are:
	* `never`
	* `less`
	* `equal`
	* `less_or_equal`
	* `greater`
	* `not_equal`
	* `greater_or_equal`
	* `always`
* `depth_bounds_test_enable`: set to `true` to limit the depth range.
* `stencil_test_enable`: set to `true` to enable the stencil test.
* `min_depth_bounds`: set to a floating point value for the minimum depth value when limiting the depth range.
* `max_depth_bounds`: set to a floating point value for the maximum depth value when limiting the depth range.

#### Stencil states

The stencil render states can be prefixed with `front_` or `back_` to only apply to the front or back stencil. For example: `front_stencil_fail_op`.

* `stencil_fail_op`: set to the stencil operation to apply when a fragment fails the stencil test. Possible values are:
	* `keep`
	* `zero`
	* `replace`
	* `increment_and_clamp`
	* `increment_and_clamp`
	* `invert`
	* `increment_and_wrap`
	* `decrement_and_wrap`
* `stencil_pass_op`: set to the stencil operation to apply when a fragment passes the stencil test. Possible values are the same as `stencil_fail_op`.
* `stencil_depth_fail_op`: set to the stencil operation to apply when a fragment fails the depth test. Possible values are the same as `stencil_fail_op`.
* `stencil_compare_op`: set to the compare operation for the stencil value. Possible values are the same as `depth_compare_op`.
* `stencil_compare_mask`: set to an integer bitmask to apply to the stencil value before the comparison. The value should be a maximum of 16 bits.
* `stencil_write_mask`: set to an integer bitmask to apply to the stencil value before writing to the stencil buffer. The value should be a maximum of 16 bits.
* `stencil_reference`: set to an integer bitmask to apply to use as the reference value. The value should be a maximum of 16 bits.

### Blend states

* `logical_op_enable`: set to `true` to apply a logical operation during alpha blending.
* `logical_op`: set to the logical operation to apply. Possible values are:
	* `clear`: clear the value to 0
	* `and`: evaluate `a & b`
	* `and_reverse`: evaluate `a & ~b`
	* `copy`: copy `a` to `b`
	* `and_inverted`: evaluate `~a & b`
	* `no_op`: don't modify the value
	* `xor`: evaluate `a ^ b`
	* `or`: evaluate `a | b`
	* `nor`: evaluate `~(a | b)`
	* `equivalent`: evaluate `~(a ^ b)`
	* `invert`: evaluate `~b`
	* `or_reverse`: evaluate `a | ~b`
	* `copy_inverted`: evaluate `~a`
	* `or_inverted`: evaluate `~a | b`
	* `nand`: evaluate `~(a & b)`
	* `set`: set the value to all 1
* `separate_attachment_blending_enable`: set to `true` to use separate blending states for each color attachment.
* `blend_constants`: set to a `vec4` value for the constant color and alpha values to use for blend operations.

#### Blend attachment states

The blend attachment states may be prefixed with `attachment#_` to apply to a specific attachment. For example: `attachment3_blend_enable`. If the attachment index is omitted, the states will apply to blend attachment 0.

* `blend_enable`: set to `true` to enable alpha blending.
* `src_color_blend_factor`: set to the factor to use for the source color. Possible values are:
	* `zero`
	* `one`
	* `src_color`
	* `one_minus_src_color`
	* `dst_color`
	* `one_minus_dst_color`
	* `src_alpha`
	* `one_minus_src_alpha`
	* `dst_alpha`
	* `one_minus_dst_alpha`
	* `src_alpha_saturate`
	* `src1_color`
	* `one_minus_src1_color`
	* `src1_alpha`
	* `one_minus_src1_alpha`
* `dst_color_blend_factor`: set to the factor to use for the destination color. Possible values are the same as `src_color_blend_factor`.
* `color_blend_op`: the operation to apply for color blending. Possible values are:
	* `add`
	* `subtract`
	* `reverse_subtract`
	* `min`
	* `max`
* `src_alpha_blend_factor`: set to the factor to use for the source alpha. Possible values are the same as `src_color_blend_factor`.
* `dst_alpha_blend_factor`: set to the factor to use for the destination alpha. Possible values are the same as `src_color_blend_factor`.
* `alpha_blend_op`: the operation to apply for alpha blending. Possible values are the same as `color_blend_op`.
* `src_blend_factor`: set to the factor to use for both the source color and alpha. Possible values are the same as `src_color_blend_factor`.
* `dst_blend_factor`: set to the factor to use for both the destination color and alpha. Possible values are the same as `src_color_blend_factor`.
* `blend_op`: the operation to apply for both the color and alpha blending. Possible values are the same as `color_blend_op`.
* `color_write_mask`: mask for the color channels to write to. This may be set to 0 to disable writing to the color buffer, or a combination of `R`, `G`, `B`, and `A` for writing to the red, green, blue, and alpha colors. For example: `RGB` to write to all channels except for alpha.

### Other render states

* `patch_control_points`: set to an integer value of the number of control points for tessellation patches.
* `early_fragment_tests`: set to `true` to enable running of depth/stencil tests before running the fragment shader. This will also write the depth value before running the shader, so any modifications of the depth within the shader will be ignored. Similarly, if the fragment is discarded the depth value will not be discarded.
