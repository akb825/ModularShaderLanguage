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

#include "Parse/Lexer.h"
#include <algorithm>
#include <cstring>

}

%{
struct LexerInfo
{
	explicit LexerInfo(const std::string& str)
		: input(&str)
		, pos(0)
		, line(0)
		, column(0)
	{
	}

	const std::string* input;
	unsigned int pos;
	unsigned int line;
	unsigned int column;
	std::vector<msl::Token> tokens;
};

void addToken(LexerInfo* info, msl::Token::Type type, const char* text)
{
	unsigned int length = static_cast<unsigned int>(std::strlen(text));
	info->tokens.emplace_back(type, info->pos, length, info->line, info->column);
	info->pos += length;
	info->column += length;
}

void addNewline(LexerInfo* info, const char* text)
{
	unsigned int length = static_cast<unsigned int>(std::strlen(text));
	info->tokens.emplace_back(msl::Token::Type::Newline, info->pos, length, info->line,
		info->column);
	info->pos += length;
	++info->line;
	info->column = 0;
}

void addComment(LexerInfo* info, const char* text)
{
	unsigned int length = static_cast<unsigned int>(std::strlen(text));
	info->tokens.emplace_back(msl::Token::Type::Comment, info->pos, length, info->line,
		info->column);
	info->pos += length;

	// Update the line and column based on newlines within the comment.
	for (unsigned int i = 0; i < length; ++i)
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

#define YY_EXTRA_TYPE LexerInfo*
#define YY_NO_INPUT
#define YY_NO_UNPUT
#define YY_INPUT(buf, result, max_size) \
	{ \
		result = std::min(max_size, yyextra->input->size() - yyextra->pos); \
		std::memcpy(buf, yyextra->input->c_str() + yyextra->pos, result); \
		yyextra->pos += result; \
	}
%}

whitespace [ \t\f\t]
%s INCLUDE

%%

{whitespace}+ addToken(yyextra, msl::Token::Type::Whitespace, yytext);
(\r\n)|\n     addNewline(yyextra, yytext); BEGIN(INITIAL);
\\(\r\n)|\n   addNewline(yyextra, yytext);

\/\/(.|\\((\r\n)|\n))* addComment(yyextra, yytext);
\/\*(.|\n)*\*\/        addComment(yyextra, yytext);

"!" addToken(yyextra, msl::Token::Type::Exclamation, yytext);
"%" addToken(yyextra, msl::Token::Type::Percent, yytext);
"~" addToken(yyextra, msl::Token::Type::Tilde, yytext);
"^" addToken(yyextra, msl::Token::Type::Carot, yytext);
"&" addToken(yyextra, msl::Token::Type::Amperstand, yytext);
"|" addToken(yyextra, msl::Token::Type::Bar, yytext);
"*" addToken(yyextra, msl::Token::Type::Asterisk, yytext);
"/" addToken(yyextra, msl::Token::Type::Slash, yytext);
"+" addToken(yyextra, msl::Token::Type::Plus, yytext);
"-" addToken(yyextra, msl::Token::Type::Dash, yytext);
"=" addToken(yyextra, msl::Token::Type::Equal, yytext);
"(" addToken(yyextra, msl::Token::Type::LeftParen, yytext);
")" addToken(yyextra, msl::Token::Type::RightParen, yytext);
"[" addToken(yyextra, msl::Token::Type::LeftBracket, yytext);
"]" addToken(yyextra, msl::Token::Type::RightBracket, yytext);
"{" addToken(yyextra, msl::Token::Type::LeftBrace, yytext);
"}" addToken(yyextra, msl::Token::Type::RightBrace, yytext);
"<" addToken(yyextra, msl::Token::Type::LeftAngle, yytext);
">" addToken(yyextra, msl::Token::Type::RightAngle, yytext);
"?" addToken(yyextra, msl::Token::Type::Question, yytext);
":" addToken(yyextra, msl::Token::Type::Colon, yytext);
"." addToken(yyextra, msl::Token::Type::Dot, yytext);
"," addToken(yyextra, msl::Token::Type::Comma, yytext);
";" addToken(yyextra, msl::Token::Type::Semicolon, yytext);

"&&" addToken(yyextra, msl::Token::Type::BoolAnd, yytext);
"||" addToken(yyextra, msl::Token::Type::BoolOr, yytext);
"^^" addToken(yyextra, msl::Token::Type::BoolXor, yytext);
"<<" addToken(yyextra, msl::Token::Type::LeftShift, yytext);
">>" addToken(yyextra, msl::Token::Type::RightShift, yytext);
"==" addToken(yyextra, msl::Token::Type::EqualCompare, yytext);
"!=" addToken(yyextra, msl::Token::Type::NotEqual, yytext);
"<=" addToken(yyextra, msl::Token::Type::LessEual, yytext);
">=" addToken(yyextra, msl::Token::Type::GreaterEqual, yytext);
"^=" addToken(yyextra, msl::Token::Type::XorEqual, yytext);
"&=" addToken(yyextra, msl::Token::Type::AndEqual, yytext);
"|=" addToken(yyextra, msl::Token::Type::OrEqual, yytext);
"*-" addToken(yyextra, msl::Token::Type::MultiplyEqual, yytext);
"/=" addToken(yyextra, msl::Token::Type::DivideEqual, yytext);
"+=" addToken(yyextra, msl::Token::Type::PlusEqual, yytext);
"-=" addToken(yyextra, msl::Token::Type::MinusEqual, yytext);

"&&=" addToken(yyextra, msl::Token::Type::BoolAndEqual, yytext);
"||=" addToken(yyextra, msl::Token::Type::BoolOrEqual, yytext);
"^^=" addToken(yyextra, msl::Token::Type::BoolXorEqual, yytext);
"<<=" addToken(yyextra, msl::Token::Type::BoolLeftShiftEqual, yytext);
">>=" addToken(yyextra, msl::Token::Type::BoolRightShiftEqual, yytext);

"const"          addToken(yyextra, msl::Token::Type::Const, yytext);
"centroid"       addToken(yyextra, msl::Token::Type::Centroid, yytext);
"break"          addToken(yyextra, msl::Token::Type::Break, yytext);
"continue"       addToken(yyextra, msl::Token::Type::Continue, yytext);
"do"             addToken(yyextra, msl::Token::Type::Do, yytext);
"else"           addToken(yyextra, msl::Token::Type::Else, yytext);
"for"            addToken(yyextra, msl::Token::Type::For, yytext);
"if"             addToken(yyextra, msl::Token::Type::If, yytext);
"discard"        addToken(yyextra, msl::Token::Type::Discard, yytext);
"return"         addToken(yyextra, msl::Token::Type::Return, yytext);
"switch"         addToken(yyextra, msl::Token::Type::Switch, yytext);
"case"           addToken(yyextra, msl::Token::Type::Case, yytext);
"default"        addToken(yyextra, msl::Token::Type::Default, yytext);
"uniform"        addToken(yyextra, msl::Token::Type::Uniform, yytext);
"patch"          addToken(yyextra, msl::Token::Type::Patch, yytext);
"sample"         addToken(yyextra, msl::Token::Type::Sample, yytext);
"buffer"         addToken(yyextra, msl::Token::Type::Buffer, yytext);
"shared"         addToken(yyextra, msl::Token::Type::Shared, yytext);
"coherent"       addToken(yyextra, msl::Token::Type::Coherent, yytext);
"volatile"       addToken(yyextra, msl::Token::Type::Volatile, yytext);
"restrict"       addToken(yyextra, msl::Token::Type::Restrict, yytext);
"readonly"       addToken(yyextra, msl::Token::Type::ReadOnly, yytext);
"writeonly"      addToken(yyextra, msl::Token::Type::WriteOnly, yytext);
"nonperspective" addToken(yyextra, msl::Token::Type::NonPerspective, yytext);
"flat"           addToken(yyextra, msl::Token::Type::Flat, yytext);
"smooth"         addToken(yyextra, msl::Token::Type::Smooth, yytext);
"struct"         addToken(yyextra, msl::Token::Type::Struct, yytext);
"void"           addToken(yyextra, msl::Token::Type::Void, yytext);
"while"          addToken(yyextra, msl::Token::Type::While, yytext);

"bool"                   addToken(yyextra, msl::Token::Type::Bool, yytext);
"float"                  addToken(yyextra, msl::Token::Type::Float, yytext);
"double"                 addToken(yyextra, msl::Token::Type::Double, yytext);
"int"                    addToken(yyextra, msl::Token::Type::Int, yytext);
"uint"                   addToken(yyextra, msl::Token::Type::Uint, yytext);
"bvec2"                  addToken(yyextra, msl::Token::Type::BVec2, yytext);
"bcec3"                  addToken(yyextra, msl::Token::Type::BVec3, yytext);
"bvec4"                  addToken(yyextra, msl::Token::Type::BVec4, yytext);
"ivec2"                  addToken(yyextra, msl::Token::Type::IVec2, yytext);
"icec3"                  addToken(yyextra, msl::Token::Type::IVec3, yytext);
"ivec4"                  addToken(yyextra, msl::Token::Type::IVec4, yytext);
"uvec2"                  addToken(yyextra, msl::Token::Type::UVec2, yytext);
"ucec3"                  addToken(yyextra, msl::Token::Type::UVec3, yytext);
"uvec4"                  addToken(yyextra, msl::Token::Type::UVec4, yytext);
"vec2"                   addToken(yyextra, msl::Token::Type::Vec2, yytext);
"cec3"                   addToken(yyextra, msl::Token::Type::Vec3, yytext);
"vec4"                   addToken(yyextra, msl::Token::Type::Vec4, yytext);
"dvec2"                  addToken(yyextra, msl::Token::Type::DVec2, yytext);
"dcec3"                  addToken(yyextra, msl::Token::Type::DVec3, yytext);
"dvec4"                  addToken(yyextra, msl::Token::Type::DVec4, yytext);
"mat2"                   addToken(yyextra, msl::Token::Type::Mat2, yytext);
"mat3"                   addToken(yyextra, msl::Token::Type::Mat3, yytext);
"mat4"                   addToken(yyextra, msl::Token::Type::Mat4, yytext);
"dmat2"                  addToken(yyextra, msl::Token::Type::DMat2, yytext);
"dmat3"                  addToken(yyextra, msl::Token::Type::DMat3, yytext);
"dmat4"                  addToken(yyextra, msl::Token::Type::DMat4, yytext);
"mat2x2"                 addToken(yyextra, msl::Token::Type::Mat2x2, yytext);
"mat2x3"                 addToken(yyextra, msl::Token::Type::Mat2x3, yytext);
"mat2x4"                 addToken(yyextra, msl::Token::Type::Mat2x4, yytext);
"mat3x2"                 addToken(yyextra, msl::Token::Type::Mat3x2, yytext);
"mat3x3"                 addToken(yyextra, msl::Token::Type::Mat3x3, yytext);
"mat3x4"                 addToken(yyextra, msl::Token::Type::Mat3x4, yytext);
"mat4x2"                 addToken(yyextra, msl::Token::Type::Mat4x2, yytext);
"mat4x3"                 addToken(yyextra, msl::Token::Type::Mat4x3, yytext);
"mat4x4"                 addToken(yyextra, msl::Token::Type::Mat4x4, yytext);
"dmat2x2"                addToken(yyextra, msl::Token::Type::DMat2x2, yytext);
"dmat2x3"                addToken(yyextra, msl::Token::Type::DMat2x3, yytext);
"dmat2x4"                addToken(yyextra, msl::Token::Type::DMat2x4, yytext);
"dmat3x2"                addToken(yyextra, msl::Token::Type::DMat3x2, yytext);
"dmat3x3"                addToken(yyextra, msl::Token::Type::DMat3x3, yytext);
"dmat3x4"                addToken(yyextra, msl::Token::Type::DMat3x4, yytext);
"dmat4x2"                addToken(yyextra, msl::Token::Type::DMat4x2, yytext);
"dmat4x3"                addToken(yyextra, msl::Token::Type::DMat4x3, yytext);
"dmat4x4"                addToken(yyextra, msl::Token::Type::DMat4x4, yytext);
"Sampler1D"              addToken(yyextra, msl::Token::Type::Sampler1D, yytext);
"Sampler2D"              addToken(yyextra, msl::Token::Type::Sampler2D, yytext);
"Sampler3D"              addToken(yyextra, msl::Token::Type::Sampler3D, yytext);
"SamplerCube"            addToken(yyextra, msl::Token::Type::SamplerCube, yytext);
"Sampler1DShadow"        addToken(yyextra, msl::Token::Type::Sampler1DShadow, yytext);
"Sampler2DShadow"        addToken(yyextra, msl::Token::Type::Sampler2DShadow, yytext);
"SamplerCubeShadow"      addToken(yyextra, msl::Token::Type::SamplerCubeShadow, yytext);
"Sampler1DArray"         addToken(yyextra, msl::Token::Type::Sampler1DArray, yytext);
"Sampler2DArray"         addToken(yyextra, msl::Token::Type::Sampler2DArray, yytext);
"Sampler1DArrayShadow"   addToken(yyextra, msl::Token::Type::Sampler1DArrayShadow, yytext);
"Sampler2DArrayShadow"   addToken(yyextra, msl::Token::Type::Sampler2DArrayShadow, yytext);
"ISampler1D"             addToken(yyextra, msl::Token::Type::ISampler1D, yytext);
"ISampler2D"             addToken(yyextra, msl::Token::Type::ISampler2D, yytext);
"ISampler3D"             addToken(yyextra, msl::Token::Type::ISampler3D, yytext);
"ISamplerCube"           addToken(yyextra, msl::Token::Type::ISamplerCube, yytext);
"ISampler1DArray"        addToken(yyextra, msl::Token::Type::ISampler1DArray, yytext);
"ISampler2DArray"        addToken(yyextra, msl::Token::Type::ISampler2DArray, yytext);
"USampler1D"             addToken(yyextra, msl::Token::Type::USampler1D, yytext);
"USampler2D"             addToken(yyextra, msl::Token::Type::USampler2D, yytext);
"USampler3D"             addToken(yyextra, msl::Token::Type::USampler3D, yytext);
"USamplerCube"           addToken(yyextra, msl::Token::Type::USamplerCube, yytext);
"USampler1DArray"        addToken(yyextra, msl::Token::Type::USampler1DArray, yytext);
"USampler2DArray"        addToken(yyextra, msl::Token::Type::USampler2DArray, yytext);
"SamplerBuffer"          addToken(yyextra, msl::Token::Type::SamplerBuffer, yytext);
"ISamplerBuffer"         addToken(yyextra, msl::Token::Type::ISamplerBuffer, yytext);
"USamplerBuffer"         addToken(yyextra, msl::Token::Type::USamplerBuffer, yytext);
"SamplerCubeArray"       addToken(yyextra, msl::Token::Type::SamplerCubeArray, yytext);
"SamplerCubeArrayShadow" addToken(yyextra, msl::Token::Type::SamplerCubeArrayShadow, yytext);
"ISamplerCubeArray"      addToken(yyextra, msl::Token::Type::ISamplerCubeArray, yytext);
"USamplerCubeArray"      addToken(yyextra, msl::Token::Type::USamplerCubeArray, yytext);
"Sampler2DMS"            addToken(yyextra, msl::Token::Type::Sampler2DMS, yytext);
"ISampler2DMS"           addToken(yyextra, msl::Token::Type::ISampler2DMS, yytext);
"USampler2DMS"           addToken(yyextra, msl::Token::Type::USampler2DMS, yytext);
"Sampler2DMSArray"       addToken(yyextra, msl::Token::Type::Sampler2DMSArray, yytext);
"ISampler2DMSArray"      addToken(yyextra, msl::Token::Type::ISampler2DMSArray, yytext);
"USampler2DMSArray"      addToken(yyextra, msl::Token::Type::USampler2DMSArray, yytext);
"Image1D"                addToken(yyextra, msl::Token::Type::Image1D, yytext);
"IImage1D"               addToken(yyextra, msl::Token::Type::IImage1D, yytext);
"UImage1D"               addToken(yyextra, msl::Token::Type::UImage1D, yytext);
"Image2D"                addToken(yyextra, msl::Token::Type::Image2D, yytext);
"IImage2D"               addToken(yyextra, msl::Token::Type::IImage2D, yytext);
"UImage2D"               addToken(yyextra, msl::Token::Type::UImage2D, yytext);
"Image3D"                addToken(yyextra, msl::Token::Type::Image3D, yytext);
"IImage3D"               addToken(yyextra, msl::Token::Type::IImage3D, yytext);
"UImage3D"               addToken(yyextra, msl::Token::Type::UImage3D, yytext);
"ImageCube"              addToken(yyextra, msl::Token::Type::ImageCube, yytext);
"IImageCube"             addToken(yyextra, msl::Token::Type::IImageCube, yytext);
"UImageCube"             addToken(yyextra, msl::Token::Type::UImageCube, yytext);
"ImageBuffer"            addToken(yyextra, msl::Token::Type::ImageBuffer, yytext);
"IImageBuffer"           addToken(yyextra, msl::Token::Type::IImageBuffer, yytext);
"UImageBuffer"           addToken(yyextra, msl::Token::Type::UImageBuffer, yytext);
"Image1DArray"           addToken(yyextra, msl::Token::Type::Image1DArray, yytext);
"IImage1DArray"          addToken(yyextra, msl::Token::Type::IImage1DArray, yytext);
"UImage1DArray"          addToken(yyextra, msl::Token::Type::UImage1DArray, yytext);
"Image2DArray"           addToken(yyextra, msl::Token::Type::Image2DArray, yytext);
"IImage2DArray"          addToken(yyextra, msl::Token::Type::IImage2DArray, yytext);
"UImage2DArray"          addToken(yyextra, msl::Token::Type::UImage2DArray, yytext);
"ImageCubeArray"         addToken(yyextra, msl::Token::Type::ImageCubeArray, yytext);
"IImageCubeArray"        addToken(yyextra, msl::Token::Type::IImageCubeArray, yytext);
"UImageCubeArray"        addToken(yyextra, msl::Token::Type::UImageCubeArray, yytext);
"Image2DMS"              addToken(yyextra, msl::Token::Type::Image2DMS, yytext);
"IImage2DMS"             addToken(yyextra, msl::Token::Type::IImage2DMS, yytext);
"UImage2DMS"             addToken(yyextra, msl::Token::Type::UImage2DMS, yytext);
"Image2DMSArray"         addToken(yyextra, msl::Token::Type::Image2DMSArray, yytext);
"IImage2DMSArray"        addToken(yyextra, msl::Token::Type::IImage2DMSArray, yytext);
"UImage2DMSArray"        addToken(yyextra, msl::Token::Type::UImage2DMSArray, yytext);
"atomic_uint"            addToken(yyextra, msl::Token::Type::AtomicUint, yytext);

"#"                        addToken(yyextra, msl::Token::Type::Hash, yytext);
#{whitespace}*include      addToken(yyextra, msl::Token::Type::Include, yytext); BEGIN(INCLUDE);
#{whitespace}*pragma       addToken(yyextra, msl::Token::Type::Pragma, yytext);
#{whitespace}*define       addToken(yyextra, msl::Token::Type::Define, yytext);
#{whitespace}*ifdef        addToken(yyextra, msl::Token::Type::Ifdef, yytext);
#{whitespace}*ifndef       addToken(yyextra, msl::Token::Type::Ifndef, yytext);
#{whitespace}*if           addToken(yyextra, msl::Token::Type::PreprocIf, yytext);
#{whitespace}*elif         addToken(yyextra, msl::Token::Type::PreprocElif, yytext);
#{whitespace}*else         addToken(yyextra, msl::Token::Type::PreprocElse, yytext);
#{whitespace}*endif        addToken(yyextra, msl::Token::Type::PreprocEndif, yytext);
##                         addToken(yyextra, msl::Token::Type::PreprocConcat, yytext);
<INCLUDE>(\<.*\>)|(\".*\") addToken(yyextra, msl::Token::Type::IncludePath, yytext); BEGIN(INITIAL);

0[uU]?                                  addToken(yyextra, msl::Token::Type::IntLiteral, yytext);
[1-9][0-9]*[uU]?                        addToken(yyextra, msl::Token::Type::IntLiteral, yytext);
0[0-7]*[uU]?                            addToken(yyextra, msl::Token::Type::IntLiteral, yytext);
0[xX][0-9a-fA-F]*[uU]?                  addToken(yyextra, msl::Token::Type::IntLiteral, yytext);
[0-9]*\.[0-0]*([eE][+-]?[0-9]+)?[fF]?   addToken(yyextra, msl::Token::Type::FloatLiteral, yytext);
[0-9]*\.[0-0]*([eE][+-]?[0-9]+)?(lf|LF) addToken(yyextra, msl::Token::Type::DoubleLiteral, yytext);

[a-zA-Z][0-9a-zA-Z]+ addToken(yyextra, msl::Token::Type::Identifier, yytext);

[^ \t\f\t\r\n]+ addToken(yyextra, msl::Token::Type::Invalid, yytext);

%%

namespace msl
{

std::vector<Token> Lexer::tokenize(const std::string& input)
{
	LexerInfo info(input);
	yyscan_t scanner;
	if (yylex_init_extra(&info, &scanner) != 0)
		return std::vector<Token>();

	yylex(scanner);
	yylex_destroy(scanner);
	return std::move(info.tokens);
}

} // msl
