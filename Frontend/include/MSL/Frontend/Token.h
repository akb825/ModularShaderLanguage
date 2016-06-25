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

#include <MSL/Frontend/Config.h>
#include <cassert>
#include <string>

/**
 * @file
 * @brief File containing a structure for a token when a file is tokenized.
 */

namespace msl
{

/**
 * @brief Structure that holds the information about a token.
 */
struct Token
{
	/**
	 * @brief The type of the token.
	 */
	enum class Type
	{
		// Non-code
		Whitespace,     ///< Whitespace, including spaces and tabs.
		Newline,        ///< Newline, separated from standard whitespace.
		EscapedNewline, ///< Newline escaped by \\, which will be removed in preprocessing.
		Comment,        ///< Code comment.

		// Single symbols
		Exclamation,  ///< @c !
		Percent,      ///< @c %
		Tilde,        ///< @c ~
		Carot,        ///< @c ^
		Amperstand,   ///< @c &
		Bar,          ///< @c |
		Asterisk,     ///< @c *
		Slash,        ///< @c /
		Plus,         ///< @c +
		Dash,         ///< @c -
		Equal,        ///< @c =
		LeftParen,    ///< @c (
		RightParen,   ///< @c )
		LeftBracket,  ///< @c [
		RightBracket, ///< @c ]
		LeftBrace,    ///< @c {
		RightBrace,   ///< @c }
		LeftAngle,    ///< @c <
		RightAngle,   ///< @c >
		Question,     ///< @c ?
		Colon,        ///< @c :
		Dot,          ///< @c .
		Comma,        ///< @c ,
		Semicolon,    ///< @c :

		// Double symbols
		BoolAnd,       ///< @c &&
		BoolOr,        ///< @c ||
		BoolXor,       ///< @c ^^
		LeftShift,     ///< @c <<
		RightShift,    ///< @c >>
		EqualCompare,  ///< @c ==
		NotEqual,      ///< @c !=
		LessEual,      ///< @c <=
		GreaterEqual,  ///< @c >=
		XorEqual,      ///< @c ^=
		AndEqual,      ///< @c &=
		OrEqual,       ///< @c |=
		MultiplyEqual, ///< @c *=
		DivideEqual,   ///< @c /=
		PlusEqual,     ///< @c +=
		MinusEqual,    ///< @c -=

		// Tripple symbols
		BoolAndEqual,        ///< @c &&=
		BoolOrEqual,         ///< @c ||=
		BoolXorEqual,        ///< @c ^^=
		BoolLeftShiftEqual,  ///< @c <<=
		BoolRightShiftEqual, ///< @c >>=

		// Keywords
		Const,          ///< @c const
		Centroid,       ///< @c centroid
		Break,          ///< @c break
		Continue,       ///< @c continue
		Do,             ///< @c do
		Else,           ///< @c else
		For,            ///< @c for
		If,             ///< @c if
		Discard,        ///< @c discard
		Return,         ///< @c return
		Switch,         ///< @c switch
		Case,           ///< @c case
		Default,        ///< @c default
		Uniform,        ///< @c uniform
		Patch,          ///< @c patch
		Sample,         ///< @c sample
		Buffer,         ///< @c buffer
		Shared,         ///< @c shared
		Coherent,       ///< @c coherent
		Volatile,       ///< @c volatile
		Restrict,       ///< @c restrict
		ReadOnly,       ///< @c readonly
		WriteOnly,      ///< @c writeonly
		NonPerspective, ///< @c nonperspective
		Flat,           ///< @c flat
		Smooth,         ///< @c smooth
		Struct,         ///< @c struct
		Void,           ///< @c void
		While,          ///< @c while

		// Types
		Bool,                   ///< @c bool
		Float,                  ///< @c float
		Double,                 ///< @c double
		Int,                    ///< @c int
		Uint,                   ///< @c uint
		BVec2,                  ///< @c bvec2
		BVec3,                  ///< @c bcec3
		BVec4,                  ///< @c bvec4
		IVec2,                  ///< @c ivec2
		IVec3,                  ///< @c icec3
		IVec4,                  ///< @c ivec4
		UVec2,                  ///< @c uvec2
		UVec3,                  ///< @c ucec3
		UVec4,                  ///< @c uvec4
		Vec2,                   ///< @c vec2
		Vec3,                   ///< @c cec3
		Vec4,                   ///< @c vec4
		DVec2,                  ///< @c dvec2
		DVec3,                  ///< @c dcec3
		DVec4,                  ///< @c dvec4
		Mat2,                   ///< @c mat2
		Mat3,                   ///< @c mat3
		Mat4,                   ///< @c mat4
		DMat2,                  ///< @c dmat2
		DMat3,                  ///< @c dmat3
		DMat4,                  ///< @c dmat4
		Mat2x2,                 ///< @c mat2x2
		Mat2x3,                 ///< @c mat2x3
		Mat2x4,                 ///< @c mat2x4
		Mat3x2,                 ///< @c mat3x2
		Mat3x3,                 ///< @c mat3x3
		Mat3x4,                 ///< @c mat3x4
		Mat4x2,                 ///< @c mat4x2
		Mat4x3,                 ///< @c mat4x3
		Mat4x4,                 ///< @c mat4x4
		DMat2x2,                ///< @c dmat2x2
		DMat2x3,                ///< @c dmat2x3
		DMat2x4,                ///< @c dmat2x4
		DMat3x2,                ///< @c dmat3x2
		DMat3x3,                ///< @c dmat3x3
		DMat3x4,                ///< @c dmat3x4
		DMat4x2,                ///< @c dmat4x2
		DMat4x3,                ///< @c dmat4x3
		DMat4x4,                ///< @c dmat4x4
		Sampler1D,              ///< @c Sampler1D
		Sampler2D,              ///< @c Sampler2D
		Sampler3D,              ///< @c Sampler3D
		SamplerCube,            ///< @c SamplerCube
		Sampler1DShadow,        ///< @c Sampler1DShadow
		Sampler2DShadow,        ///< @c Sampler2DShadow
		SamplerCubeShadow,      ///< @c SamplerCubeShadow
		Sampler1DArray,         ///< @c Sampler1DArray
		Sampler2DArray,         ///< @c Sampler2DArray
		Sampler1DArrayShadow,   ///< @c Sampler1DArrayShadow
		Sampler2DArrayShadow,   ///< @c Sampler2DArrayShadow
		ISampler1D,             ///< @c ISampler1D
		ISampler2D,             ///< @c ISampler2D
		ISampler3D,             ///< @c ISampler3D
		ISamplerCube,           ///< @c ISamplerCube
		ISampler1DArray,        ///< @c ISampler1DArray
		ISampler2DArray,        ///< @c ISampler2DArray
		USampler1D,             ///< @c USampler1D
		USampler2D,             ///< @c USampler2D
		USampler3D,             ///< @c USampler3D
		USamplerCube,           ///< @c USamplerCube
		USampler1DArray,        ///< @c USampler1DArray
		USampler2DArray,        ///< @c USampler2DArray
		SamplerBuffer,          ///< @c SamplerBuffer
		ISamplerBuffer,         ///< @c ISamplerBuffer
		USamplerBuffer,         ///< @c USamplerBuffer
		SamplerCubeArray,       ///< @c SamplerCubeArray
		SamplerCubeArrayShadow, ///< @c SamplerCubeArrayShadow
		ISamplerCubeArray,      ///< @c ISamplerCubeArray
		USamplerCubeArray,      ///< @c USamplerCubeArray
		Sampler2DMS,            ///< @c Sampler2DMS
		ISampler2DMS,           ///< @c ISampler2DMS
		USampler2DMS,           ///< @c USampler2DMS
		Sampler2DMSArray,       ///< @c Sampler2DMSArray
		ISampler2DMSArray,      ///< @c ISampler2DMSArray
		USampler2DMSArray,      ///< @c USampler2DMSArray
		Image1D,                ///< @c Image1D
		IImage1D,               ///< @c IImage1D
		UImage1D,               ///< @c UImage1D
		Image2D,                ///< @c Image2D
		IImage2D,               ///< @c IImage2D
		UImage2D,               ///< @c UImage2D
		Image3D,                ///< @c Image3D
		IImage3D,               ///< @c IImage3D
		UImage3D,               ///< @c UImage3D
		ImageCube,              ///< @c ImageCube
		IImageCube,             ///< @c IImageCube
		UImageCube,             ///< @c UImageCube
		ImageBuffer,            ///< @c ImageBuffer
		IImageBuffer,           ///< @c IImageBuffer
		UImageBuffer,           ///< @c UImageBuffer
		Image1DArray,           ///< @c Image1DArray
		IImage1DArray,          ///< @c IImage1DArray
		UImage1DArray,          ///< @c UImage1DArray
		Image2DArray,           ///< @c Image2DArray
		IImage2DArray,          ///< @c IImage2DArray
		UImage2DArray,          ///< @c UImage2DArray
		ImageCubeArray,         ///< @c ImageCubeArray
		IImageCubeArray,        ///< @c IImageCubeArray
		UImageCubeArray,        ///< @c UImageCubeArray
		Image2DMS,              ///< @c Image2DMS
		IImage2DMS,             ///< @c IImage2DMS
		UImage2DMS,             ///< @c UImage2DMS
		Image2DMSArray,         ///< @c Image2DMSArray
		IImage2DMSArray,        ///< @c IImage2DMSArray
		UImage2DMSArray,        ///< @c UImage2DMSArray
		AtomicUint,             ///< @c atomic_uint

		// Pre-processor
		Hash,         ///< @c \#
		Include,      ///< @c \#include
		Pragma,       ///< @c \#pragma
		Define,       ///< @c \#define
		Ifdef,        ///< @c \#ifdef
		Ifndef,       ///< @c \#ifndef
		PreprocIf,    ///< @c \#if
		PreprocElif,  ///< @c \#elif
		PreprocElse,  ///< @c \#else
		PreprocEndif, ///< @c \#endif
		IncludePath,  ///< Include path surrounded by "" or <>.

		// Literals
		IntLiteral,    ///< Integer literal.
		FloatLiteral,  ///< Float literal.
		DoubleLiteral, ///< Double literal.

		// Other
		Identifier, ///< Any other identifier.
		Invalid,    ///< An invalid token.
	};

	/**
	 * @brief Category of a token.
	 */
	enum class Category
	{
		Whitespace,   ///< Whitespace, including newlines and escaped newlines.
		Comment,      ///< Code comment.
		Symbol,       ///< Symbols and operators.
		Keyword,      ///< Non-type keywords.
		Type,         ///< Built-in type.
		Preprocessor, ///< Pre-processor directive.
		Number,       ///< Integer or float literal.
		Identifier, ///< Any other identifier.
		Invalid,    ///< An invalid token.
	};

	/**
	 * @brief Gets the category for a token type.
	 * @param type The token type.
	 * @return The token category.
	 */
	static Category getCategory(Type type);

	/**
	 * @brief Default constructor.
	 */
	Token();

	/**
	 * @brief Constructs the token.
	 * @param type_ The type of the token.
	 * @param start_ The start index of the token in the string.
	 * @param length_ The length of the token.
	 * @param line_ The line of the token.
	 * @param column_ The column of the token in the line.
	 */
	Token(Type type_, unsigned int start_, unsigned int length_, unsigned int line_,
		unsigned int column_);

	/**
	 * @brief the type of the token.
	 */
	Type type;

	/**
	 * @brief The start index of the token in the string.
	 */
	unsigned int start;

	/**
	 * @brief The length of the token.
	 */
	unsigned int length;

	/**
	 * @brief The line number of te token.
	 */
	unsigned int line;

	/**
	 * @brief The column of the token within the line.
	 */
	unsigned int column;
};

inline Token::Category Token::getCategory(Type type)
{
	switch (type)
	{
		case Type::Whitespace:
		case Type::Newline:
		case Type::EscapedNewline:
			return Category::Whitespace;

		case Type::Comment:
			return Category::Comment;

		case Type::Exclamation:
		case Type::Percent:
		case Type::Tilde:
		case Type::Carot:
		case Type::Amperstand:
		case Type::Bar:
		case Type::Asterisk:
		case Type::Slash:
		case Type::Plus:
		case Type::Dash:
		case Type::Equal:
		case Type::LeftParen:
		case Type::RightParen:
		case Type::LeftBracket:
		case Type::RightBracket:
		case Type::LeftBrace:
		case Type::RightBrace:
		case Type::LeftAngle:
		case Type::RightAngle:
		case Type::Question:
		case Type::Colon:
		case Type::Dot:
		case Type::Comma:
		case Type::Semicolon:
		case Type::BoolAnd:
		case Type::BoolOr:
		case Type::BoolXor:
		case Type::LeftShift:
		case Type::RightShift:
		case Type::EqualCompare:
		case Type::NotEqual:
		case Type::LessEual:
		case Type::GreaterEqual:
		case Type::XorEqual:
		case Type::AndEqual:
		case Type::OrEqual:
		case Type::MultiplyEqual:
		case Type::DivideEqual:
		case Type::PlusEqual:
		case Type::MinusEqual:
		case Type::BoolAndEqual:
		case Type::BoolOrEqual:
		case Type::BoolXorEqual:
		case Type::BoolLeftShiftEqual:
		case Type::BoolRightShiftEqual:
			return Category::Symbol;

		case Type::Const:
		case Type::Centroid:
		case Type::Break:
		case Type::Continue:
		case Type::Do:
		case Type::Else:
		case Type::For:
		case Type::If:
		case Type::Discard:
		case Type::Return:
		case Type::Switch:
		case Type::Case:
		case Type::Default:
		case Type::Uniform:
		case Type::Patch:
		case Type::Sample:
		case Type::Buffer:
		case Type::Shared:
		case Type::Coherent:
		case Type::Volatile:
		case Type::Restrict:
		case Type::ReadOnly:
		case Type::WriteOnly:
		case Type::NonPerspective:
		case Type::Flat:
		case Type::Smooth:
		case Type::Struct:
		case Type::Void:
		case Type::While:
			return Category::Keyword;

		case Type::Bool:
		case Type::Float:
		case Type::Double:
		case Type::Int:
		case Type::Uint:
		case Type::BVec2:
		case Type::BVec3:
		case Type::BVec4:
		case Type::IVec2:
		case Type::IVec3:
		case Type::IVec4:
		case Type::UVec2:
		case Type::UVec3:
		case Type::UVec4:
		case Type::Vec2:
		case Type::Vec3:
		case Type::Vec4:
		case Type::DVec2:
		case Type::DVec3:
		case Type::DVec4:
		case Type::Mat2:
		case Type::Mat3:
		case Type::Mat4:
		case Type::DMat2:
		case Type::DMat3:
		case Type::DMat4:
		case Type::Mat2x2:
		case Type::Mat2x3:
		case Type::Mat2x4:
		case Type::Mat3x2:
		case Type::Mat3x3:
		case Type::Mat3x4:
		case Type::Mat4x2:
		case Type::Mat4x3:
		case Type::Mat4x4:
		case Type::DMat2x2:
		case Type::DMat2x3:
		case Type::DMat2x4:
		case Type::DMat3x2:
		case Type::DMat3x3:
		case Type::DMat3x4:
		case Type::DMat4x2:
		case Type::DMat4x3:
		case Type::DMat4x4:
		case Type::Sampler1D:
		case Type::Sampler2D:
		case Type::Sampler3D:
		case Type::SamplerCube:
		case Type::Sampler1DShadow:
		case Type::Sampler2DShadow:
		case Type::SamplerCubeShadow:
		case Type::Sampler1DArray:
		case Type::Sampler2DArray:
		case Type::Sampler1DArrayShadow:
		case Type::Sampler2DArrayShadow:
		case Type::ISampler1D:
		case Type::ISampler2D:
		case Type::ISampler3D:
		case Type::ISamplerCube:
		case Type::ISampler1DArray:
		case Type::ISampler2DArray:
		case Type::USampler1D:
		case Type::USampler2D:
		case Type::USampler3D:
		case Type::USamplerCube:
		case Type::USampler1DArray:
		case Type::USampler2DArray:
		case Type::SamplerBuffer:
		case Type::ISamplerBuffer:
		case Type::USamplerBuffer:
		case Type::SamplerCubeArray:
		case Type::SamplerCubeArrayShadow:
		case Type::ISamplerCubeArray:
		case Type::USamplerCubeArray:
		case Type::Sampler2DMS:
		case Type::ISampler2DMS:
		case Type::USampler2DMS:
		case Type::Sampler2DMSArray:
		case Type::ISampler2DMSArray:
		case Type::USampler2DMSArray:
		case Type::Image1D:
		case Type::IImage1D:
		case Type::UImage1D:
		case Type::Image2D:
		case Type::IImage2D:
		case Type::UImage2D:
		case Type::Image3D:
		case Type::IImage3D:
		case Type::UImage3D:
		case Type::ImageCube:
		case Type::IImageCube:
		case Type::UImageCube:
		case Type::ImageBuffer:
		case Type::IImageBuffer:
		case Type::UImageBuffer:
		case Type::Image1DArray:
		case Type::IImage1DArray:
		case Type::UImage1DArray:
		case Type::Image2DArray:
		case Type::IImage2DArray:
		case Type::UImage2DArray:
		case Type::ImageCubeArray:
		case Type::IImageCubeArray:
		case Type::UImageCubeArray:
		case Type::Image2DMS:
		case Type::IImage2DMS:
		case Type::UImage2DMS:
		case Type::Image2DMSArray:
		case Type::IImage2DMSArray:
		case Type::UImage2DMSArray:
		case Type::AtomicUint:
			return Category::Type;

		case Type::Hash:
		case Type::Include:
		case Type::Pragma:
		case Type::Define:
		case Type::Ifdef:
		case Type::Ifndef:
		case Type::PreprocIf:
		case Type::PreprocElif:
		case Type::PreprocElse:
		case Type::PreprocEndif:
		case Type::IncludePath:
			return Category::Preprocessor;

		case Type::IntLiteral:
		case Type::FloatLiteral:
		case Type::DoubleLiteral:
			return Category::Number;

		case Type::Identifier:
			return Category::Identifier;

		case Type::Invalid:
			return Category::Invalid;

		default:
			assert(false);
			return Category::Invalid;
	}
}

inline Token::Token()
	: type(Type::Invalid)
	, start(0)
	, length(0)
	, line(0)
	, column(0)
{
}

inline Token::Token(Type type_, unsigned int start_, unsigned int length_, unsigned int line_,
	unsigned int column_)
	: type(type_)
	, start(start_)
	, length(length_)
	, line(line_)
	, column(column_)
{
}

} // namespace msl
