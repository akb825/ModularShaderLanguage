in vec4 vtcPosition[];
in vec3
		vtcNormal
		[];
in vec2 vtcTexCoord1[];
in vec2 vtcTexCoord2[];
in Block
	{
		vec3 vtcTangent;
		vec3 vtcBinormal;
	};
out vec4 tcePosition[];
out patch vec3 tceNormal;
out patch vec2 tceTexCoord1;
out patch vec2 tceTexCoord2;
out patch Block
	{
		vec3 tceTangent;
		vec3 tceBinormal;
	};
void main()
{
}
