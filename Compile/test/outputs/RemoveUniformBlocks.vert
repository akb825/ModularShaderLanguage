uniform sampler2D testTexture;
layout(push_constant) 

 uniform TestBlock1
{
	layout(position=10) vec4 uniform1;
	vec3 uniform2 = {0, 1, 2};
	uniform vec2 uniform3;
} testBlock;
layout(push_constant) uniform TestBlock2
{
	layout(position=10) vec4 uniform4;
	vec3 uniform5 = {0, 1, 2};
	uniform vec2 uniform6;
};
