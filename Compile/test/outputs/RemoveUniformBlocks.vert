precision mediump float;
struct TestStruct
{
	vec3 value1;
	int value2;
};
layout(push_constant) uniform Uniforms
{
uniform vec4 testFreeUniform1;
uniform float testFreeUniform2;
uniform TestStruct testFreeUniform3;
uniform 
ivec3
testFreeUniform4;
	layout(position=10) vec4 uniform1;
	vec3 uniform2 = {0, 1, 2};
	uniform vec2 uniform3;
	layout(position=10) vec4 uniform4;
	vec3 uniform5 = {0, 1, 2};
	uniform vec2
		uniform6;
} uniforms;
uniform sampler2D testTexture;
uniform iimage2D testImage;
