in vec4 tcePosition[];
in patch vec3 tceNormal;
in patch vec2 tceTexCoord1;
in patch vec2 tceTexCoord2;
in patch Block
	{
		vec3 tceTangent;
		vec3 tceBinormal;
	};
out vec4 tefPosition;
out vec3 tefNormal;
out vec2 tefTexCoord1;
out vec2 tefTexCoord2;
out Block
	{
		vec3 tefTangent;
		vec3 tefBinormal;
	};
void main()
{
}
