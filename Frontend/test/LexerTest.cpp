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

#include "TokenHelpers.h"
#include <MSL/Frontend/Parse/Lexer.h>
#include <gtest/gtest.h>

namespace msl
{

TEST(LexerTest, Whitespace)
{
	std::string input = " \t\n\t \\\n \t\r\n\t \\\r\n \t\f\v";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Whitespace, " \t");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Whitespace, "\t ");
	addToken(expectedTokens, input, Token::Type::EscapedNewline, "\\\n");
	addToken(expectedTokens, input, Token::Type::Whitespace, " \t");
	addToken(expectedTokens, input, Token::Type::Newline, "\r\n");
	addToken(expectedTokens, input, Token::Type::Whitespace, "\t ");
	addToken(expectedTokens, input, Token::Type::EscapedNewline, "\\\r\n");
	addToken(expectedTokens, input, Token::Type::Whitespace, " \t\f\v");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, Comment)
{
	std::string input =
		"\t// C++ style comment\n"
		"// C++ style comment with \\\n"
		" escaped newline.\n"
		"// C++ style comment with \\\r\n"
		" escaped carriage return newline.\r\n"
		"  /*\n"
		"   * C style comment.\r\n"
		"   */\n"
		"/* Unterminated";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Whitespace, "\t");
	addToken(expectedTokens, input, Token::Type::Comment, "// C++ style comment");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Comment,
		"// C++ style comment with \\\n"
		" escaped newline.");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Comment,
		"// C++ style comment with \\\r\n"
		" escaped carriage return newline.");
	addToken(expectedTokens, input, Token::Type::Newline, "\r\n");
	addToken(expectedTokens, input, Token::Type::Whitespace, "  ");
	addToken(expectedTokens, input, Token::Type::Comment,
		"/*\n"
		"   * C style comment.\r\n"
		"   */");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Slash, "/");
	addToken(expectedTokens, input, Token::Type::Asterisk, "*");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Identifier, "Unterminated");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, SingleSymbols)
{
	std::string input = "!%~^&|*/+-(=)[]{}<>?:.,;";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Exclamation, "!");
	addToken(expectedTokens, input, Token::Type::Percent, "%");
	addToken(expectedTokens, input, Token::Type::Tilde, "~");
	addToken(expectedTokens, input, Token::Type::Carot, "^");
	addToken(expectedTokens, input, Token::Type::Amperstand, "&");
	addToken(expectedTokens, input, Token::Type::Bar, "|");
	addToken(expectedTokens, input, Token::Type::Asterisk, "*");
	addToken(expectedTokens, input, Token::Type::Slash, "/");
	addToken(expectedTokens, input, Token::Type::Plus, "+");
	addToken(expectedTokens, input, Token::Type::Dash, "-");
	addToken(expectedTokens, input, Token::Type::LeftParen, "(");
	addToken(expectedTokens, input, Token::Type::Equal, "=");
	addToken(expectedTokens, input, Token::Type::RightParen, ")");
	addToken(expectedTokens, input, Token::Type::LeftSquare, "[");
	addToken(expectedTokens, input, Token::Type::RightSquare, "]");
	addToken(expectedTokens, input, Token::Type::LeftBrace, "{");
	addToken(expectedTokens, input, Token::Type::RightBrace, "}");
	addToken(expectedTokens, input, Token::Type::LeftAngle, "<");
	addToken(expectedTokens, input, Token::Type::RightAngle, ">");
	addToken(expectedTokens, input, Token::Type::Question, "?");
	addToken(expectedTokens, input, Token::Type::Colon, ":");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Comma, ",");
	addToken(expectedTokens, input, Token::Type::Semicolon, ";");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, DoubleSymbols)
{
	std::string input = "&&||^^<<>>!===<=>=^=&=|=*=/=+=-=";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::BoolAnd, "&&");
	addToken(expectedTokens, input, Token::Type::BoolOr, "||");
	addToken(expectedTokens, input, Token::Type::BoolXor, "^^");
	addToken(expectedTokens, input, Token::Type::LeftShift, "<<");
	addToken(expectedTokens, input, Token::Type::RightShift, ">>");
	addToken(expectedTokens, input, Token::Type::NotEqual, "!=");
	addToken(expectedTokens, input, Token::Type::EqualCompare, "==");
	addToken(expectedTokens, input, Token::Type::LessEqual, "<=");
	addToken(expectedTokens, input, Token::Type::GreaterEqual, ">=");
	addToken(expectedTokens, input, Token::Type::XorEqual, "^=");
	addToken(expectedTokens, input, Token::Type::AndEqual, "&=");
	addToken(expectedTokens, input, Token::Type::OrEqual, "|=");
	addToken(expectedTokens, input, Token::Type::MultiplyEqual, "*=");
	addToken(expectedTokens, input, Token::Type::DivideEqual, "/=");
	addToken(expectedTokens, input, Token::Type::PlusEqual, "+=");
	addToken(expectedTokens, input, Token::Type::MinusEqual, "-=");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, TrippleSymbols)
{
	std::string input = "&&=||=^^=<<=>>=";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::BoolAndEqual, "&&=");
	addToken(expectedTokens, input, Token::Type::BoolOrEqual, "||=");
	addToken(expectedTokens, input, Token::Type::BoolXorEqual, "^^=");
	addToken(expectedTokens, input, Token::Type::LeftShiftEqual, "<<=");
	addToken(expectedTokens, input, Token::Type::RightShiftEqual, ">>=");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, Keyword)
{
	std::string input = "const centroid break continue do else for if discard return switch case "
		"default uniform patch sample buffer shared coherent volatile restrict readonly writeonly "
		"nonperspective flat smooth struct void while true false";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Const, "const");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Centroid, "centroid");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Break, "break");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Continue, "continue");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Do, "do");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Else, "else");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::For, "for");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::If, "if");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Discard, "discard");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Return, "return");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Switch, "switch");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Case, "case");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Default, "default");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Uniform, "uniform");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Patch, "patch");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sample, "sample");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Buffer, "buffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Shared, "shared");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Coherent, "coherent");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Volatile, "volatile");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Restrict, "restrict");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ReadOnly, "readonly");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::WriteOnly, "writeonly");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::NonPerspective, "nonperspective");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Flat, "flat");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Smooth, "smotth");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Struct, "struct");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Void, "void");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::While, "while");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::True, "true");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::False, "false");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, KeywordNoSpace)
{
	std::string input = "constbreak";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Identifier, "constbreak");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, ScalarTypes)
{
	std::string input = "bool float double int uint atomic_uint";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Bool, "bool");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Float, "float");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Double, "double");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Int, "int");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UInt, "uint");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::AtomicUInt, "atomic_uint");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, VectorTypes)
{
	std::string input = "bvec2 bvec3 bvec4 ivec2 ivec3 ivec4 uvec2 uvec3 uvec4 vec2 vec3 vec4 "
		"dvec2 dvec3 dvec4";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::BVec2, "bvec2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::BVec3, "bvec3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::BVec4, "bvec4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IVec2, "ivec2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IVec3, "ivec3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IVec4, "ivec4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UVec2, "uvec2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UVec3, "uvec3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UVec4, "uvec4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Vec2, "vec2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Vec3, "vec3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Vec4, "vec4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DVec2, "dvec2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DVec3, "dvec3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DVec4, "dvec4");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, MatrixTypes)
{
	std::string input = "mat2 mat3 mat4 dmat2 dmat3 dmat4 mat2x2 mat2x3 mat2x4 "
		"mat3x2 mat3x3 mat3x4 mat4x2 mat4x3 mat4x4 dmat2x2 dmat2x3 dmat2x4 "
		"dmat3x2 dmat3x3 dmat3x4 dmat4x2 dmat4x3 dmat4x4";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Mat2, "mat2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat3, "mat3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat4, "mat4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat2, "dmat2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat3, "dmat3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat4, "dmat4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat2x2, "mat2x2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat2x3, "mat2x3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat2x4, "mat2x4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat3x2, "mat3x2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat3x3, "mat3x3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat3x4, "mat3x4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat4x2, "mat4x2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat4x3, "mat4x3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Mat4x4, "mat4x4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat2x2, "dmat2x2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat2x3, "dmat2x3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat2x4, "dmat2x4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat3x2, "dmat3x2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat3x3, "dmat3x3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat3x4, "dmat3x4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat4x2, "dmat4x2");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat4x3, "dmat4x3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DMat4x4, "dmat4x4");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, SamplerTypes)
{
	std::string input = "sampler1D sampler2D sampler3D samplerCube sampler1DShadow sampler2DShadow "
		"samplerCubeShadow sampler1DArray sampler2DArray sampler1DArrayShadow sampler2DArrayShadow "
		"isampler1D isampler2D isampler3D isamplerCube isampler1DArray isampler2DArray usampler1D "
		"usampler2D usampler3D usamplerCube usampler1DArray usampler2DArray samplerBuffer "
		"isamplerBuffer usamplerBuffer samplerCubeArray samplerCubeArrayShadow isamplerCubeArray "
		"usamplerCubeArray sampler2DMS isampler2DMS usampler2DMS sampler2DMSArray "
		"isampler2DMSArray usampler2DMSArray";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Sampler1D, "sampler1D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler2D, "sampler2D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler3D, "sampler3D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::SamplerCube, "samplerCube");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler1DShadow, "sampler1DShadow");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler2DShadow, "sampler2DShadow");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::SamplerCubeShadow, "samplerCubeShadow");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler1DArray, "sampler1DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler2DArray, "sampler2DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler1DArrayShadow, "sampler1DArrayShadow");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler2DArrayShadow, "sampler2DArrayShadow");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler1D, "isampler1D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler2D, "isampler2D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler3D, "isampler3D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISamplerCube, "isamplerCube");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler1DArray, "isampler1DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler2DArray, "isampler2DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler1D, "usampler1D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler2D, "usampler2D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler3D, "usampler3D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USamplerCube, "usamplerCube");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler1DArray, "usampler1DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler2DArray, "usampler2DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::SamplerBuffer, "samplerBuffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISamplerBuffer, "isamplerBuffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USamplerBuffer, "usamplerBuffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::SamplerCubeArray, "samplerCubeArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::SamplerCubeArrayShadow, "samplerCubeArrayShadow");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISamplerCubeArray, "isamplerCubeArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USamplerCubeArray, "usamplerCubeArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler2DMS, "sampler2DMS");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler2DMS, "isampler2DMS");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler2DMS, "usampler2DMS");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Sampler2DMSArray, "sampler2DMSArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ISampler2DMSArray, "isampler2DMSArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::USampler2DMSArray, "usampler2DMSArray");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, ImageTypes)
{
	std::string input = "image1D iimage1D uimage1D image2D iimage2D uimage2D image3D iimage3D "
		"uimage3D imageCube iimageCube uimageCube imageBuffer iimageBuffer uimageBuffer "
		"image1DArray iimage1DArray uimage1DArray image2DArray iimage2DArray uimage2DArray "
		"imageCubeArray iimageCubeArray uimageCubeArray image2DMS iimage2DMS uimage2DMS "
		"image2DMSArray iimage2DMSArray uimage2DMSArray";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Image1D, "image1D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage1D, "iimage1D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage1D, "uimage1D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Image2D, "image2D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage2D, "iimage2D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage2D, "uimage2D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Image3D, "image3D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage3D, "iimage3D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage3D, "uimage3D");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ImageCube, "imageCube");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImageCube, "iimageCube");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImageCube, "uimageCube");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ImageBuffer, "imageBuffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImageBuffer, "iimageBuffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImageBuffer, "uimageBuffer");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Image1DArray, "image1DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage1DArray, "iimage1DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage1DArray, "uimage1DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Image2DArray, "image2DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage2DArray, "iimage2DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage2DArray, "uimage2DArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::ImageCubeArray, "imageCubeArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImageCubeArray, "iimageCubeArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImageCubeArray, "uimageCubeArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Image2DMS, "image2DMS");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage2DMS, "iimage2DMS");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage2DMS, "uimage2DMS");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Image2DMSArray, "image2DMSArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IImage2DMSArray, "iimage2DMSArray");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::UImage2DMSArray, "uimage2DMSArray");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, PreProcessor)
{
	std::string input = "# #pragma #define #undef #ifdef #ifndef #if #elif #else #endif ## "
		"# pragma # define # undef # ifdef # ifndef # if # elif # else # endif";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Hash, "#");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Pragma, "#pragma");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Define, "#define");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Undef, "#undef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Ifdef, "#ifdef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Ifndef, "#ifndef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocIf, "#if");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocElif, "#elif");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocElse, "#else");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocEndif, "#endif");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocConcat, "##");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Pragma, "# pragma");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Define, "# define");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Undef, "# undef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Ifdef, "# ifdef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Ifndef, "# ifndef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocIf, "# if");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocElif, "# elif");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocElse, "# else");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::PreprocEndif, "# endif");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, Include)
{
	std::string input = "#include <test file.h>\n"
		"# include \"test file.h\"\n"
		"#include \\\n"
		" <test.h>\n"
		"#include \\\r\n"
		"\"test.h\"\r\n"
		"\"test.h\"\n"
		"#include\n"
		"<test.h>\n"
		"#include <test.h> \"test.h\"\n"
		"#include <test.h\"\n"
		"#include \"test.h>";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IncludePath, "<test file.h>");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "# include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IncludePath, "\"test file.h\"");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::EscapedNewline, "\\\n");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IncludePath, "<test.h>");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::EscapedNewline, "\\\r\n");
	addToken(expectedTokens, input, Token::Type::IncludePath, "\"test.h\"");
	addToken(expectedTokens, input, Token::Type::Newline, "\r\n");
	addToken(expectedTokens, input, Token::Type::Invalid, "\"test");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Invalid, "h\"");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::LeftAngle, "<");
	addToken(expectedTokens, input, Token::Type::Identifier, "test");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "h");
	addToken(expectedTokens, input, Token::Type::RightAngle, ">");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IncludePath, "<test.h>");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "\"test");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Invalid, "h\"");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::LeftAngle, "<");
	addToken(expectedTokens, input, Token::Type::Identifier, "test");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Invalid, "h\"");
	addToken(expectedTokens, input, Token::Type::Newline, "\n");
	addToken(expectedTokens, input, Token::Type::Include, "#include");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "\"test");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "h");
	addToken(expectedTokens, input, Token::Type::RightAngle, ">");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, IntLiteral)
{
	std::string input = "0 0u 0U 1234567890 1234567890u 1234567890U 01234567 01234567u 01234567U "
		"0x1234567890abcdef 0x1234567890abcdefu 0X1234567890ABCDEFU 0123456789 12u34 012u34 "
		"0x12u34";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::IntLiteral, "0");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "0u");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "0U");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "1234567890");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "1234567890u");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "1234567890U");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "01234567");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "01234567u");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "01234567U");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "0x1234567890abcdef");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "0x1234567890abcdefu");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::IntLiteral, "0X1234567890ABCDEFU");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "0123456789");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "12u34");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "012u34");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "0x12u34");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, FloatLiteral)
{
	std::string input = "0. 0.f 0.F .0 .0f .0F 1.23 1.23f 1.23F 1.23e4 1.23e+4 1.23e-4 1.23e4f "
		"1.23e+4f 1.23e-4f 1.23E4F 1.23E+4F 1.23E-4F 0f .f .e3 a1.0e3";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "0.");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "0.f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "0.F");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, ".0");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, ".0f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, ".0F");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23F");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23e4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23e+4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23e-4");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23e4f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23e+4f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23e-4f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23E4F");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23E+4F");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, "1.23E-4F");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "0f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "f");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "e3");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Identifier, "a1");
	addToken(expectedTokens, input, Token::Type::FloatLiteral, ".0e3");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, DoubleLiteral)
{
	std::string input = "0.lf 0.LF .0lf .0LF 1.23lf 1.23LF 1.23e4lf 1.23e+4lf 1.23e-4lf 1.23E4LF "
		"1.23E+4LF 1.23E-4LF 0lf .lf .e3lf a1.0e3lf";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "0.lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "0.LF");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, ".0lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, ".0LF");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23LF");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23e4lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23e+4lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23e-4lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23E4LF");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23E+4LF");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, "1.23E-4LF");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "0lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "e3lf");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Identifier, "a1");
	addToken(expectedTokens, input, Token::Type::DoubleLiteral, ".0e3lf");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, Identifier)
{
	std::string input = "az09AZ _a_z0_9A_Z 09azAZ a.b0-c1+d3";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	std::vector<Token> expectedTokens;
	addToken(expectedTokens, input, Token::Type::Identifier, "az09AZ");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Identifier, "_a_z0_9A_Z");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Invalid, "09azAZ");
	addToken(expectedTokens, input, Token::Type::Whitespace, " ");
	addToken(expectedTokens, input, Token::Type::Identifier, "a");
	addToken(expectedTokens, input, Token::Type::Dot, ".");
	addToken(expectedTokens, input, Token::Type::Identifier, "b0");
	addToken(expectedTokens, input, Token::Type::Dash, "-");
	addToken(expectedTokens, input, Token::Type::Identifier, "c1");
	addToken(expectedTokens, input, Token::Type::Plus, "+");
	addToken(expectedTokens, input, Token::Type::Identifier, "d3");

	EXPECT_EQ(expectedTokens, tokens);
}

TEST(LexerTest, AddMessage)
{
	std::string file = "test.msl";
	std::string input = "az09AZ\n"
		"_a_z0_9A_Z 09azAZ a.b0-c1+d3";
	std::vector<Token> tokens = Lexer::tokenize(0, input);

	Output output;
	for (const Token& token : tokens)
		token.addMessage(output, file, input);

	const std::vector<Output::Message>& messages = output.getMessages();
	ASSERT_EQ(1U, messages.size());
	EXPECT_EQ(Output::Level::Error, messages[0].level);
	EXPECT_EQ(file, messages[0].file);
	EXPECT_EQ(1, messages[0].line);
	EXPECT_EQ(11, messages[0].column);
	EXPECT_FALSE(messages[0].continued);
	EXPECT_EQ("Invalid token '09azAZ'", messages[0].message);
}

} // namespace msl
