struct StructAll
{
	vec4 value;
};
struct InstanceStructAll
{
	vec4 value;
} structInstanceAll;
 struct StructGeometry
{
	vec4 value;
};
 struct InstanceStructGeometry
{
	vec4 value;
} structInstanceGeometry;
layout (std140)
uniform UniformBlockAll
{
	vec4 uniformBlockVarAll;
};
uniform UniformBlockInstanceAll
{
	vec4 instance;
} uniformInstanceAll;
 layout (std140)
uniform UniformBlockGeometry
{
	vec4 uniformBlockVarGeometry;
};
 uniform UniformBlockInstanceGeometry
{
	vec4 instance;
} uniformInstanceGeometry;
int globalVarAll;
in vec2 inVarAll;
out vec3 outVarAll;
uniform sampler2D unfiformVarAll;
in inBufferAll
{
	vec2 inBufferVarAll;
};
in inBufferAll
{
	vec2 instance;
} inInstanceAll;
out outBufferAll
{
	vec2 outBufferVarAll;
};
out outBufferAll
{
	vec2 instance;
} outInstanceAll;
out vec3 outVarAll;
layout (std140)
buffer BufferAll
{
	vec4 bufferVarAll;
};
buffer BufferInstanceAll
{
	vec4 instance;
} bufferInstanceAll;
void funcDeclAll(in vec3 input, out vec4 output);
void funcAll()
{
	if (true)
	{
		gl_Position = bufferVarAll;
	}
}
int globalVarGeometry;
in vec2 inVarGeometry;
 out vec3 outVarGeometry;
uniform sampler2D unfiformVarGeometry;
 in inBufferGeometry
{
	vec2 inBufferVarGeometry;
};
 in inBufferGeometry
{
	vec2 instance;
} inInstanceGeometry;
 out outBufferGeometry
{
	vec2 outBufferVarGeometry;
};
 out outBufferGeometry
{
	vec2 instance;
} outInstanceGeometry;
 out vec3 outVarGeometry;
 layout (std140)
buffer BufferGeometry
{
	vec4 bufferVarGeometry;
};
 buffer BufferInstanceGeometry
{
	vec4 instance;
} bufferInstanceGeometry;
 void funcDeclGeometry(in vec3 input, out vec4 output);
 void funcGeometry()
{
	if (true)
	{
		gl_Position = bufferVarGeometry;
	}
}
