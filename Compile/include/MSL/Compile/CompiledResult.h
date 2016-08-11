/*
 * Copyright 2016 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <MSL/Compile/Config.h>
#include <MSL/Compile/Export.h>
#include <cstdint>
#include <map>
#include <vector>

namespace msl
{

class Target;

/**
 * @brief Class that stores the compiled result for a shader.
 *
 * This contains the compiled result and reflection information for the uniforms and buffers
 * for each pipeline.
 */
class MSL_COMPILE_EXPORT CompiledResult
{
public:
	/**
	 * @brief Constant for no shader being set.
	 */
	static const std::size_t noShader = static_cast<std::size_t>(-1);

	/**
	 * @brief Constant for no known value.
	 */
	static const std::uint32_t unknown = static_cast<std::uint32_t>(-1);

	/**
	 * @brief Enum for the type of a uniform.
	 */
	enum class Type
	{
		// Scalars and vectors
		Float,  ///< float
		Vec2,   ///< vec2
		Vec3,   ///< vec3
		Vec4,   ///< vec4
		Double, ///< double
		DVec2,  ///< dvec2
		DVec3,  ///< dvec3
		DVec4,  ///< dvec4
		Int,    ///< int
		IVec2,  ///< ivec2
		IVec3,  ///< ivec3
		IVec4,  ///< ivec4
		UInt,   ///< unsigned int
		UVec2,  ///< uvec2
		UVec3,  ///< uvec3
		UVec4,  ///< uvec4
		Bool,   ///< bool
		BVec2,  ///< bvec2
		BVec3,  ///< bvec3
		BVec4,  ///< bvec4

		// Matrices
		Mat2,    ///< mat2, mat2x2
		Mat3,    ///< mat3, mat3x3
		Mat4,    ///< mat4, mat4x4
		Mat2x3,  ///< mat2x3
		Mat2x4,  ///< mat2x4
		Mat3x2,  ///< mat3x2
		Mat3x4,  ///< mat3x4
		Mat4x2,  ///< mat4x2
		Mat4x3,  ///< mat4x3
		DMat2,   ///< dmat2, dmat2x2
		DMat3,   ///< dmat3, dmat3x3
		DMat4,   ///< dmat4, dmat4x4
		DMat2x3, ///< dmat2x3
		DMat2x4, ///< dmat2x4
		DMat3x2, ///< dmat3x2
		DMat3x4, ///< dmat3x4
		DMat4x2, ///< dmat4x2
		DMat4x3, ///< dmat4x3

		// Samplers
		Sampler1D,            ///< sampler1D
		Sampler2D,            ///< sampler2D
		Sampler3D,            ///< sampler3D
		SamplerCube,          ///< samplerCube
		Sampler1DShadow,      ///< sampler1DShadow
		Sampler2DShadow,      ///< sampler2DShadow
		Sampler1DArray,       ///< sampler1DArray
		Sampler2DArray,       ///< sampler2DArray
		Sampler1DArrayShadow, ///< sampler1DArrayShadow
		Sampler2DArrayShadow, ///< sampler2DArrayShadow
		Sampler2DMS,          ///< sampler2DMS
		Sampler2DMSArray,     ///< sampler2DMSArray
		SamplerCubeShadow,    ///< samplerCubeShadow
		SamplerBuffer,        ///< samplerBuffer
		Sampler2DRect,        ///< sampler2DRect
		Sampler2DRectShadow,  ///< sampler2DRectShadow
		ISampler1D,           ///< isampler1D
		ISampler2D,           ///< isampler2D
		ISampler3D,           ///< isampler3D
		ISamplerCube,         ///< isamplerCube
		ISampler1DArray,      ///< isampler1DArray
		ISampler2DArray,      ///< isampler2DArray
		ISampler2DMS,         ///< isampler2DMS
		ISampler2DMSArray,    ///< isampler2DMSArray
		ISampler2DRect,       ///< isampler2DRect
		USampler1D,           ///< usampler1D
		USampler2D,           ///< usampler2D
		USampler3D,           ///< usampler3D
		USamplerCube,         ///< usamplerCube
		USampler1DArray,      ///< usampler1DArray
		USampler2DArray,      ///< usampler2DArray
		USampler2DMS,         ///< usampler2DMS
		USampler2DMSArray,    ///< usampler2DMSArray
		USampler2DRect,       ///< usampler2DRect
	};

	/**
	 * @brief Structure describing a uniform.
	 *
	 * This includes free-floating uniforms, elements of a uniform buffer, or elements of a storage
	 * buffer.
	 */
	struct Uniform
	{
		/**
		 * @brief The name of the uniform.
		 */
		std::string name;

		/**
		 * @brief The type of the uniform.
		 */
		Type type;

		/**
		 * @brief The index of the uniform block this belongs to.
		 *
		 * This is set to unknown when not part of a uniform block.
		 */
		std::uint32_t blockIndex;

		/**
		 * @brief The offset within the uniform block buffer.
		 *
		 * This is set to unknown if the offset isn't known or is not part of a uniform block.
		 * When the offset isn't known, it must be queried from the graphics API.
		 */
		std::uint32_t bufferOffset;

		/**
		 * @brief The number of array elements.
		 *
		 * This will be a minimum of 1.
		 */
		std::uint32_t elements;
	};

	/**
	 * @brief Structure describing a uniform block or storage buffer.
	 *
	 * This contains information about the block itself. The contents of the buffer are stored
	 * described in the list of Uniforms.
	 */
	struct UniformBlock
	{
		/**
		 * @brief The name of the block.
		 */
		std::string name;

		/**
		 * @brief The size of the block.
		 *
		 * If set to unknown, the size is unknown and must be queried by the graphics API.
		 */
		std::uint32_t size;
	};

	/**
	 * @brief Structure describing a vertex attribute.
	 */
	struct Attribute
	{
		/**
		 * @brief THe name of the attribute.
		 */
		std::string name;

		/**
		 * @brief THe type of the attribute.
		 */
		Type type;
	};

	/**
	 * @brief Structure that holds the information about the a pipeline within the compiled result.
	 */
	struct Pipeline
	{
		/**
		 * @brief Index for the shader data for the vertex stage.
		 */
		std::size_t vertex = noShader;

		/**
		 * @brief Index for the shader data for the tessellation control stage.
		 */
		std::size_t tessellationControl = noShader;

		/**
		 * @brief Index for the shader data for the tessellation evaluation stage.
		 */
		std::size_t tessellationEvaluation = noShader;

		/**
		 * @brief Index for the shader data for the geometry stage.
		 */
		std::size_t geometry = noShader;

		/**
		 * @brief Index for the shader data for the fragment stage.
		 */
		std::size_t fragment = noShader;

		/**
		 * @brief Index for the shader data for the compute stage.
		 */
		std::size_t compute = noShader;

		/**
		 * @brief The active uniforms used within the pipeline.
		 */
		std::vector<Uniform> uniforms;

		/**
		 * @brief The active uniform blocks used within the pipeline.
		 */
		std::vector<UniformBlock> uniformBlocks;

		/**
		 * @brief The active vertex attributes used within the pipeline.
		 */
		std::vector<Attribute> attributes;

	private:
		friend class Target;

		// Internal use for error reporting.
		std::string file;
		std::size_t line = 0;
		std::size_t column = 0;
	};

	CompiledResult();

	/**
	 * @brief Gets the target ID.
	 * @return The target ID.
	 */
	inline std::uint32_t getTargetId() const;

	/**
	 * @brief Gets the target version.
	 * @return The target version.
	 */
	inline std::uint32_t getTargetVersion() const;

	/**
	 * @brief Gets the shaders.
	 *
	 * The pipeline stage indices can be used to index into this vector.
	 *
	 * @return The shaders.
	 */
	inline const std::vector<std::vector<uint8_t>>& getShaders() const;

	/**
	 * @brief Gets the pipelines.
	 *
	 * The first element is the pipeline name, the second element contians the pipeline itself.
	 *
	 * @return The pipelines.
	 */
	inline const std::map<std::string, Pipeline>& getPipelines() const;

	/**
	 * @brief Gets the shared data for all the shaders.
	 * @return The shared data.
	 */
	inline const std::vector<std::uint8_t>& getSharedData() const;

private:
	friend class Target;

	std::size_t addShader(std::vector<std::uint8_t> shader);

	const Target* m_target;
	std::uint32_t m_targetId;
	std::uint32_t m_targetVersion;

	// Use a map to ensure consistent ordering.
	std::map<std::string, Pipeline> m_pipelines;
	std::vector<std::vector<std::uint8_t>> m_shaders;
	std::vector<std::uint8_t> m_sharedData;
};

inline std::uint32_t CompiledResult::getTargetId() const
{
	return m_targetId;
}

inline std::uint32_t CompiledResult::getTargetVersion() const
{
	return m_targetVersion;
}

inline const std::vector<std::vector<uint8_t>>& CompiledResult::getShaders() const
{
	return m_shaders;
}

inline const std::map<std::string, CompiledResult::Pipeline>& CompiledResult::getPipelines() const
{
	return m_pipelines;
}

inline const std::vector<std::uint8_t>& CompiledResult::getSharedData() const
{
	return m_sharedData;
}

} // namespace msl
