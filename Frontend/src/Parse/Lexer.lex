%top{
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

#include <MSL/Frontend/Parse/Lexer.h>
#include <algorithm>
#include <cstring>

#if MSL_GCC || MSL_CLANG
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#if MSL_WINDOWS
#define YY_NO_UNISTD_H
#include <io.h>
#endif

}

%{
namespace
{

struct LexerInfo
{
	explicit LexerInfo(const std::string& str, std::size_t start, std::size_t length)
		: input(&str)
		, pos(std::min(start, input->size()))
		, end(std::min(length, input->size()))
		, tokenPos(pos)
		, line(0)
		, column(0)
		, curToken(nullptr)
	{
		// Initialize the line and column number.
		for (std::size_t i = 0; i < start; ++i)
		{
			if ((*input)[i] == '\n')
			{
				++line;
				column = 0;
			}
			else
				++column;
		}
	}

	const std::string* input;
	std::size_t pos;
	std::size_t end;
	std::size_t tokenPos;
	std::size_t line;
	std::size_t column;
	msl::Token* curToken;
};

void addToken(LexerInfo* info, msl::Token::Type type, const char* text)
{
	std::size_t length = std::strlen(text);
	assert(info->curToken);
	*info->curToken = msl::Token(type, info->tokenPos, length, info->line, info->column);
	info->tokenPos += length;
	assert(info->tokenPos <= info->end);
	info->column += length;
}

void addNewline(LexerInfo* info, msl::Token::Type type, const char* text)
{
	std::size_t length = std::strlen(text);
	assert(info->curToken);
	*info->curToken = msl::Token(type, info->tokenPos, length, info->line, info->column);
	info->tokenPos += length;
	assert(info->tokenPos <= info->end);
	++info->line;
	info->column = 0;
}

void addComment(LexerInfo* info, const char* text)
{
	std::size_t length = std::strlen(text);
	assert(info->curToken);
	*info->curToken = msl::Token(msl::Token::Type::Comment, info->tokenPos, length, info->line,
		info->column);
	info->tokenPos += length;
	assert(info->tokenPos <= info->end);

	// Update the line and column based on newlines within the comment.
	for (std::size_t i = 0; i < length; ++i)
	{
		if (text[i] == '\n')
		{
			++info->line;
			info->column = 0;
		}
		else
			++info->column;
	}
}

} // namespace

#define YY_DECL static bool msllex (yyscan_t yyscanner)
#define YY_EXTRA_TYPE LexerInfo*
#define YY_NO_INPUT
#define YY_NO_UNPUT
#define YY_SKIP_YYWRAP
#define YY_INPUT(buf, result, max_size) \
	{ \
		result = std::min(max_size, yyextra->end - yyextra->pos); \
		assert(yyextra->pos + result <= yyextra->input->size()); \
		std::memcpy(buf, yyextra->input->c_str() + yyextra->pos, result); \
		yyextra->pos += result; \
	}

bool mslwrap(yyscan_t)
{
	return true;
}
%}

whitespace [ \t\f\v]
%s INCLUDE

%%

{whitespace}+ addToken(yyextra, msl::Token::Type::Whitespace, yytext); return true;
(\r\n)|\n     addNewline(yyextra, msl::Token::Type::Newline, yytext); BEGIN(INITIAL); return true;
\\((\r\n)|\n) addNewline(yyextra, msl::Token::Type::EscapedNewline, yytext); return true;

\/\/([^\r\n]|\\((\r\n)|\n))* addComment(yyextra, yytext); return true;
\/\*(.|\n)*\*\/              addComment(yyextra, yytext); return true;

"!" addToken(yyextra, msl::Token::Type::Exclamation, yytext); return true;
"%" addToken(yyextra, msl::Token::Type::Percent, yytext); return true;
"~" addToken(yyextra, msl::Token::Type::Tilde, yytext); return true;
"^" addToken(yyextra, msl::Token::Type::Carot, yytext); return true;
"&" addToken(yyextra, msl::Token::Type::Amperstand, yytext); return true;
"|" addToken(yyextra, msl::Token::Type::Bar, yytext); return true;
"*" addToken(yyextra, msl::Token::Type::Asterisk, yytext); return true;
"/" addToken(yyextra, msl::Token::Type::Slash, yytext); return true;
"+" addToken(yyextra, msl::Token::Type::Plus, yytext); return true;
"-" addToken(yyextra, msl::Token::Type::Dash, yytext); return true;
"=" addToken(yyextra, msl::Token::Type::Equal, yytext); return true;
"(" addToken(yyextra, msl::Token::Type::LeftParen, yytext); return true;
")" addToken(yyextra, msl::Token::Type::RightParen, yytext); return true;
"[" addToken(yyextra, msl::Token::Type::LeftSquare, yytext); return true;
"]" addToken(yyextra, msl::Token::Type::RightSquare, yytext); return true;
"{" addToken(yyextra, msl::Token::Type::LeftBrace, yytext); return true;
"}" addToken(yyextra, msl::Token::Type::RightBrace, yytext); return true;
"<" addToken(yyextra, msl::Token::Type::LeftAngle, yytext); return true;
">" addToken(yyextra, msl::Token::Type::RightAngle, yytext); return true;
"?" addToken(yyextra, msl::Token::Type::Question, yytext); return true;
":" addToken(yyextra, msl::Token::Type::Colon, yytext); return true;
"." addToken(yyextra, msl::Token::Type::Dot, yytext); return true;
"," addToken(yyextra, msl::Token::Type::Comma, yytext); return true;
";" addToken(yyextra, msl::Token::Type::Semicolon, yytext); return true;

"&&" addToken(yyextra, msl::Token::Type::BoolAnd, yytext); return true;
"||" addToken(yyextra, msl::Token::Type::BoolOr, yytext); return true;
"^^" addToken(yyextra, msl::Token::Type::BoolXor, yytext); return true;
"<<" addToken(yyextra, msl::Token::Type::LeftShift, yytext); return true;
">>" addToken(yyextra, msl::Token::Type::RightShift, yytext); return true;
"==" addToken(yyextra, msl::Token::Type::EqualCompare, yytext); return true;
"!=" addToken(yyextra, msl::Token::Type::NotEqual, yytext); return true;
"<=" addToken(yyextra, msl::Token::Type::LessEqual, yytext); return true;
">=" addToken(yyextra, msl::Token::Type::GreaterEqual, yytext); return true;
"^=" addToken(yyextra, msl::Token::Type::XorEqual, yytext); return true;
"&=" addToken(yyextra, msl::Token::Type::AndEqual, yytext); return true;
"|=" addToken(yyextra, msl::Token::Type::OrEqual, yytext); return true;
"*=" addToken(yyextra, msl::Token::Type::MultiplyEqual, yytext); return true;
"/=" addToken(yyextra, msl::Token::Type::DivideEqual, yytext); return true;
"+=" addToken(yyextra, msl::Token::Type::PlusEqual, yytext); return true;
"-=" addToken(yyextra, msl::Token::Type::MinusEqual, yytext); return true;

"&&=" addToken(yyextra, msl::Token::Type::BoolAndEqual, yytext); return true;
"||=" addToken(yyextra, msl::Token::Type::BoolOrEqual, yytext); return true;
"^^=" addToken(yyextra, msl::Token::Type::BoolXorEqual, yytext); return true;
"<<=" addToken(yyextra, msl::Token::Type::LeftShiftEqual, yytext); return true;
">>=" addToken(yyextra, msl::Token::Type::RightShiftEqual, yytext); return true;

"const"          addToken(yyextra, msl::Token::Type::Const, yytext); return true;
"centroid"       addToken(yyextra, msl::Token::Type::Centroid, yytext); return true;
"break"          addToken(yyextra, msl::Token::Type::Break, yytext); return true;
"continue"       addToken(yyextra, msl::Token::Type::Continue, yytext); return true;
"do"             addToken(yyextra, msl::Token::Type::Do, yytext); return true;
"else"           addToken(yyextra, msl::Token::Type::Else, yytext); return true;
"for"            addToken(yyextra, msl::Token::Type::For, yytext); return true;
"if"             addToken(yyextra, msl::Token::Type::If, yytext); return true;
"discard"        addToken(yyextra, msl::Token::Type::Discard, yytext); return true;
"return"         addToken(yyextra, msl::Token::Type::Return, yytext); return true;
"switch"         addToken(yyextra, msl::Token::Type::Switch, yytext); return true;
"case"           addToken(yyextra, msl::Token::Type::Case, yytext); return true;
"default"        addToken(yyextra, msl::Token::Type::Default, yytext); return true;
"uniform"        addToken(yyextra, msl::Token::Type::Uniform, yytext); return true;
"patch"          addToken(yyextra, msl::Token::Type::Patch, yytext); return true;
"sample"         addToken(yyextra, msl::Token::Type::Sample, yytext); return true;
"buffer"         addToken(yyextra, msl::Token::Type::Buffer, yytext); return true;
"shared"         addToken(yyextra, msl::Token::Type::Shared, yytext); return true;
"coherent"       addToken(yyextra, msl::Token::Type::Coherent, yytext); return true;
"volatile"       addToken(yyextra, msl::Token::Type::Volatile, yytext); return true;
"restrict"       addToken(yyextra, msl::Token::Type::Restrict, yytext); return true;
"readonly"       addToken(yyextra, msl::Token::Type::ReadOnly, yytext); return true;
"writeonly"      addToken(yyextra, msl::Token::Type::WriteOnly, yytext); return true;
"nonperspective" addToken(yyextra, msl::Token::Type::NonPerspective, yytext); return true;
"flat"           addToken(yyextra, msl::Token::Type::Flat, yytext); return true;
"smooth"         addToken(yyextra, msl::Token::Type::Smooth, yytext); return true;
"struct"         addToken(yyextra, msl::Token::Type::Struct, yytext); return true;
"void"           addToken(yyextra, msl::Token::Type::Void, yytext); return true;
"while"          addToken(yyextra, msl::Token::Type::While, yytext); return true;

"bool"                   addToken(yyextra, msl::Token::Type::Bool, yytext); return true;
"float"                  addToken(yyextra, msl::Token::Type::Float, yytext); return true;
"double"                 addToken(yyextra, msl::Token::Type::Double, yytext); return true;
"int"                    addToken(yyextra, msl::Token::Type::Int, yytext); return true;
"uint"                   addToken(yyextra, msl::Token::Type::UInt, yytext); return true;
"bvec2"                  addToken(yyextra, msl::Token::Type::BVec2, yytext); return true;
"bvec3"                  addToken(yyextra, msl::Token::Type::BVec3, yytext); return true;
"bvec4"                  addToken(yyextra, msl::Token::Type::BVec4, yytext); return true;
"ivec2"                  addToken(yyextra, msl::Token::Type::IVec2, yytext); return true;
"ivec3"                  addToken(yyextra, msl::Token::Type::IVec3, yytext); return true;
"ivec4"                  addToken(yyextra, msl::Token::Type::IVec4, yytext); return true;
"uvec2"                  addToken(yyextra, msl::Token::Type::UVec2, yytext); return true;
"uvec3"                  addToken(yyextra, msl::Token::Type::UVec3, yytext); return true;
"uvec4"                  addToken(yyextra, msl::Token::Type::UVec4, yytext); return true;
"vec2"                   addToken(yyextra, msl::Token::Type::Vec2, yytext); return true;
"vec3"                   addToken(yyextra, msl::Token::Type::Vec3, yytext); return true;
"vec4"                   addToken(yyextra, msl::Token::Type::Vec4, yytext); return true;
"dvec2"                  addToken(yyextra, msl::Token::Type::DVec2, yytext); return true;
"dvec3"                  addToken(yyextra, msl::Token::Type::DVec3, yytext); return true;
"dvec4"                  addToken(yyextra, msl::Token::Type::DVec4, yytext); return true;
"mat2"                   addToken(yyextra, msl::Token::Type::Mat2, yytext); return true;
"mat3"                   addToken(yyextra, msl::Token::Type::Mat3, yytext); return true;
"mat4"                   addToken(yyextra, msl::Token::Type::Mat4, yytext); return true;
"dmat2"                  addToken(yyextra, msl::Token::Type::DMat2, yytext); return true;
"dmat3"                  addToken(yyextra, msl::Token::Type::DMat3, yytext); return true;
"dmat4"                  addToken(yyextra, msl::Token::Type::DMat4, yytext); return true;
"mat2x2"                 addToken(yyextra, msl::Token::Type::Mat2x2, yytext); return true;
"mat2x3"                 addToken(yyextra, msl::Token::Type::Mat2x3, yytext); return true;
"mat2x4"                 addToken(yyextra, msl::Token::Type::Mat2x4, yytext); return true;
"mat3x2"                 addToken(yyextra, msl::Token::Type::Mat3x2, yytext); return true;
"mat3x3"                 addToken(yyextra, msl::Token::Type::Mat3x3, yytext); return true;
"mat3x4"                 addToken(yyextra, msl::Token::Type::Mat3x4, yytext); return true;
"mat4x2"                 addToken(yyextra, msl::Token::Type::Mat4x2, yytext); return true;
"mat4x3"                 addToken(yyextra, msl::Token::Type::Mat4x3, yytext); return true;
"mat4x4"                 addToken(yyextra, msl::Token::Type::Mat4x4, yytext); return true;
"dmat2x2"                addToken(yyextra, msl::Token::Type::DMat2x2, yytext); return true;
"dmat2x3"                addToken(yyextra, msl::Token::Type::DMat2x3, yytext); return true;
"dmat2x4"                addToken(yyextra, msl::Token::Type::DMat2x4, yytext); return true;
"dmat3x2"                addToken(yyextra, msl::Token::Type::DMat3x2, yytext); return true;
"dmat3x3"                addToken(yyextra, msl::Token::Type::DMat3x3, yytext); return true;
"dmat3x4"                addToken(yyextra, msl::Token::Type::DMat3x4, yytext); return true;
"dmat4x2"                addToken(yyextra, msl::Token::Type::DMat4x2, yytext); return true;
"dmat4x3"                addToken(yyextra, msl::Token::Type::DMat4x3, yytext); return true;
"dmat4x4"                addToken(yyextra, msl::Token::Type::DMat4x4, yytext); return true;
"sampler1D"              addToken(yyextra, msl::Token::Type::Sampler1D, yytext); return true;
"sampler2D"              addToken(yyextra, msl::Token::Type::Sampler2D, yytext); return true;
"sampler3D"              addToken(yyextra, msl::Token::Type::Sampler3D, yytext); return true;
"samplerCube"            addToken(yyextra, msl::Token::Type::SamplerCube, yytext); return true;
"sampler1DShadow"        addToken(yyextra, msl::Token::Type::Sampler1DShadow, yytext); return true;
"sampler2DShadow"        addToken(yyextra, msl::Token::Type::Sampler2DShadow, yytext); return true;
"samplerCubeShadow"      addToken(yyextra, msl::Token::Type::SamplerCubeShadow, yytext); return true;
"sampler1DArray"         addToken(yyextra, msl::Token::Type::Sampler1DArray, yytext); return true;
"sampler2DArray"         addToken(yyextra, msl::Token::Type::Sampler2DArray, yytext); return true;
"sampler1DArrayShadow"   addToken(yyextra, msl::Token::Type::Sampler1DArrayShadow, yytext); return true;
"sampler2DArrayShadow"   addToken(yyextra, msl::Token::Type::Sampler2DArrayShadow, yytext); return true;
"isampler1D"             addToken(yyextra, msl::Token::Type::ISampler1D, yytext); return true;
"isampler2D"             addToken(yyextra, msl::Token::Type::ISampler2D, yytext); return true;
"isampler3D"             addToken(yyextra, msl::Token::Type::ISampler3D, yytext); return true;
"isamplerCube"           addToken(yyextra, msl::Token::Type::ISamplerCube, yytext); return true;
"isampler1DArray"        addToken(yyextra, msl::Token::Type::ISampler1DArray, yytext); return true;
"isampler2DArray"        addToken(yyextra, msl::Token::Type::ISampler2DArray, yytext); return true;
"usampler1D"             addToken(yyextra, msl::Token::Type::USampler1D, yytext); return true;
"usampler2D"             addToken(yyextra, msl::Token::Type::USampler2D, yytext); return true;
"usampler3D"             addToken(yyextra, msl::Token::Type::USampler3D, yytext); return true;
"usamplerCube"           addToken(yyextra, msl::Token::Type::USamplerCube, yytext); return true;
"usampler1DArray"        addToken(yyextra, msl::Token::Type::USampler1DArray, yytext); return true;
"usampler2DArray"        addToken(yyextra, msl::Token::Type::USampler2DArray, yytext); return true;
"samplerBuffer"          addToken(yyextra, msl::Token::Type::SamplerBuffer, yytext); return true;
"isamplerBuffer"         addToken(yyextra, msl::Token::Type::ISamplerBuffer, yytext); return true;
"usamplerBuffer"         addToken(yyextra, msl::Token::Type::USamplerBuffer, yytext); return true;
"samplerCubeArray"       addToken(yyextra, msl::Token::Type::SamplerCubeArray, yytext); return true;
"samplerCubeArrayShadow" addToken(yyextra, msl::Token::Type::SamplerCubeArrayShadow, yytext); return true;
"isamplerCubeArray"      addToken(yyextra, msl::Token::Type::ISamplerCubeArray, yytext); return true;
"usamplerCubeArray"      addToken(yyextra, msl::Token::Type::USamplerCubeArray, yytext); return true;
"sampler2DMS"            addToken(yyextra, msl::Token::Type::Sampler2DMS, yytext); return true;
"isampler2DMS"           addToken(yyextra, msl::Token::Type::ISampler2DMS, yytext); return true;
"usampler2DMS"           addToken(yyextra, msl::Token::Type::USampler2DMS, yytext); return true;
"sampler2DMSArray"       addToken(yyextra, msl::Token::Type::Sampler2DMSArray, yytext); return true;
"isampler2DMSArray"      addToken(yyextra, msl::Token::Type::ISampler2DMSArray, yytext); return true;
"usampler2DMSArray"      addToken(yyextra, msl::Token::Type::USampler2DMSArray, yytext); return true;
"image1D"                addToken(yyextra, msl::Token::Type::Image1D, yytext); return true;
"iimage1D"               addToken(yyextra, msl::Token::Type::IImage1D, yytext); return true;
"uimage1D"               addToken(yyextra, msl::Token::Type::UImage1D, yytext); return true;
"image2D"                addToken(yyextra, msl::Token::Type::Image2D, yytext); return true;
"iimage2D"               addToken(yyextra, msl::Token::Type::IImage2D, yytext); return true;
"uimage2D"               addToken(yyextra, msl::Token::Type::UImage2D, yytext); return true;
"image3D"                addToken(yyextra, msl::Token::Type::Image3D, yytext); return true;
"iimage3D"               addToken(yyextra, msl::Token::Type::IImage3D, yytext); return true;
"uimage3D"               addToken(yyextra, msl::Token::Type::UImage3D, yytext); return true;
"imageCube"              addToken(yyextra, msl::Token::Type::ImageCube, yytext); return true;
"iimageCube"             addToken(yyextra, msl::Token::Type::IImageCube, yytext); return true;
"uimageCube"             addToken(yyextra, msl::Token::Type::UImageCube, yytext); return true;
"imageBuffer"            addToken(yyextra, msl::Token::Type::ImageBuffer, yytext); return true;
"iimageBuffer"           addToken(yyextra, msl::Token::Type::IImageBuffer, yytext); return true;
"uimageBuffer"           addToken(yyextra, msl::Token::Type::UImageBuffer, yytext); return true;
"image1DArray"           addToken(yyextra, msl::Token::Type::Image1DArray, yytext); return true;
"iimage1DArray"          addToken(yyextra, msl::Token::Type::IImage1DArray, yytext); return true;
"uimage1DArray"          addToken(yyextra, msl::Token::Type::UImage1DArray, yytext); return true;
"image2DArray"           addToken(yyextra, msl::Token::Type::Image2DArray, yytext); return true;
"iimage2DArray"          addToken(yyextra, msl::Token::Type::IImage2DArray, yytext); return true;
"uimage2DArray"          addToken(yyextra, msl::Token::Type::UImage2DArray, yytext); return true;
"imageCubeArray"         addToken(yyextra, msl::Token::Type::ImageCubeArray, yytext); return true;
"iimageCubeArray"        addToken(yyextra, msl::Token::Type::IImageCubeArray, yytext); return true;
"uimageCubeArray"        addToken(yyextra, msl::Token::Type::UImageCubeArray, yytext); return true;
"image2DMS"              addToken(yyextra, msl::Token::Type::Image2DMS, yytext); return true;
"iimage2DMS"             addToken(yyextra, msl::Token::Type::IImage2DMS, yytext); return true;
"uimage2DMS"             addToken(yyextra, msl::Token::Type::UImage2DMS, yytext); return true;
"image2DMSArray"         addToken(yyextra, msl::Token::Type::Image2DMSArray, yytext); return true;
"iimage2DMSArray"        addToken(yyextra, msl::Token::Type::IImage2DMSArray, yytext); return true;
"uimage2DMSArray"        addToken(yyextra, msl::Token::Type::UImage2DMSArray, yytext); return true;
"atomic_uint"            addToken(yyextra, msl::Token::Type::AtomicUInt, yytext); return true;

"#"                        addToken(yyextra, msl::Token::Type::Hash, yytext); return true;
#{whitespace}*include      addToken(yyextra, msl::Token::Type::Include, yytext); BEGIN(INCLUDE); return true;
#{whitespace}*pragma       addToken(yyextra, msl::Token::Type::Pragma, yytext); return true;
#{whitespace}*define       addToken(yyextra, msl::Token::Type::Define, yytext); return true;
#{whitespace}*ifdef        addToken(yyextra, msl::Token::Type::Ifdef, yytext); return true;
#{whitespace}*ifndef       addToken(yyextra, msl::Token::Type::Ifndef, yytext); return true;
#{whitespace}*if           addToken(yyextra, msl::Token::Type::PreprocIf, yytext); return true;
#{whitespace}*elif         addToken(yyextra, msl::Token::Type::PreprocElif, yytext); return true;
#{whitespace}*else         addToken(yyextra, msl::Token::Type::PreprocElse, yytext); return true;
#{whitespace}*endif        addToken(yyextra, msl::Token::Type::PreprocEndif, yytext); return true;
##                         addToken(yyextra, msl::Token::Type::PreprocConcat, yytext); return true;
<INCLUDE>(\<.*\>)|(\".*\") addToken(yyextra, msl::Token::Type::IncludePath, yytext); BEGIN(INITIAL); return true;

0[uU]?                                  addToken(yyextra, msl::Token::Type::IntLiteral, yytext); return true;
[1-9][0-9]*[uU]?                        addToken(yyextra, msl::Token::Type::IntLiteral, yytext); return true;
0[0-7]+[uU]?                            addToken(yyextra, msl::Token::Type::IntLiteral, yytext); return true;
0[xX][0-9a-fA-F]+[uU]?                  addToken(yyextra, msl::Token::Type::IntLiteral, yytext); return true;
[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?[fF]?   addToken(yyextra, msl::Token::Type::FloatLiteral, yytext); return true;
[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?[fF]?   addToken(yyextra, msl::Token::Type::FloatLiteral, yytext); return true;
[0-9]+\.[0-9]*([eE][+-]?[0-9]+)?(lf|LF) addToken(yyextra, msl::Token::Type::DoubleLiteral, yytext); return true;
[0-9]*\.[0-9]+([eE][+-]?[0-9]+)?(lf|LF) addToken(yyextra, msl::Token::Type::DoubleLiteral, yytext); return true;

[_a-zA-Z][_0-9a-zA-Z]* addToken(yyextra, msl::Token::Type::Identifier, yytext); return true;

[^ \t\f\t\r\n!%~^&|*/+\-=()[\]{}<>?:.,;]+ addToken(yyextra, msl::Token::Type::Invalid, yytext); return true;

%%

namespace msl
{

class Lexer::Impl
{
public:
	Impl(const std::string& input, std::size_t start, std::size_t length)
		: m_info(input, start, length)
	{
		yylex_init_extra(&m_info, &m_scanner);
	}

	~Impl()
	{
		yylex_destroy(m_scanner);
	}

	yyscan_t m_scanner;
	LexerInfo m_info;
};

std::vector<Token> Lexer::tokenize(const std::string& input, std::size_t start, std::size_t length)
{
	std::vector<Token> tokens;
	Token curToken;

	Lexer lexer(input, start, length);
	while (lexer.nextToken(curToken))
		tokens.push_back(curToken);

	return tokens;
}

Lexer::Lexer(const std::string& input, std::size_t start, std::size_t length)
	: m_impl(new Impl(input, start, length))
{
}

Lexer::~Lexer()
{
}

Lexer::Lexer(Lexer&& other)
	: m_impl(std::move(other.m_impl))
{
}

Lexer& Lexer::operator=(Lexer&& other)
{
	m_impl = std::move(other.m_impl);
	return *this;
}

bool Lexer::nextToken(Token& token)
{
	assert(m_impl);
	m_impl->m_info.curToken = &token;
	return msllex(m_impl->m_scanner);
}

} // msl
