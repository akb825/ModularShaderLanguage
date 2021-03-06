struct StructAll
{
	vec4 value;
};
struct InstanceStructAll
{
	vec4 value;
} structInstanceAll;
 struct StructFragment
{
	vec4 value;
};
 struct InstanceStructFragment
{
	vec4 value;
} structInstanceFragment;
 struct StructVertexFragment
{
	vec4 value;
};
 struct InstanceStructVertexFragment
{
	vec4 value;
} structInstanceVertexFragment;
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
uniform UniformBlockFragment
{
	vec4 uniformBlockVarFragment;
};
 uniform UniformBlockInstanceFragment
{
	vec4 instance;
} uniformInstanceFragment;
 layout (std140)
uniform UniformBlockVertexFragment
{
	vec4 uniformBlockVarVertexFragment;
};
 uniform UniformBlockInstanceVertexFragment
{
	vec4 instance;
} uniformInstanceVertexFragment;
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
int globalVarFragment;
in vec2 inVarFragment;
 out vec3 outVarFragment;
uniform sampler2D unfiformVarFragment;
 in inBufferFragment
{
	vec2 inBufferVarFragment;
};
 in inBufferFragment
{
	vec2 instance;
} inInstanceFragment;
 out outBufferFragment
{
	vec2 outBufferVarFragment;
};
 out outBufferFragment
{
	vec2 instance;
} outInstanceFragment;
 out vec3 outVarFragment;
 layout (std140)
buffer BufferFragment
{
	vec4 bufferVarFragment;
};
 buffer BufferInstanceFragment
{
	vec4 instance;
} bufferInstanceFragment;
 void funcDeclFragment(in vec3 input, out vec4 output);
 void funcFragment()
{
	if (true)
	{
		gl_Position = bufferVarFragment;
	}
}
int globalVarVertexFragment;
in vec2 inVarVertexFragment;
 out vec3 outVarVertexFragment;
uniform sampler2D unfiformVarVertexFragment;
 in inBufferVertexFragment
{
	vec2 inBufferVarVertexFragment;
};
 in inBufferVertexFragment
{
	vec2 instance;
} inInstanceVertexFragment;
 out outBufferVertexFragment
{
	vec2 outBufferVarVertexFragment;
};
 out outBufferVertexFragment
{
	vec2 instance;
} outInstanceVertexFragment;
 out vec3 outVarVertexFragment;
 layout (std140)
buffer BufferVertexFragment
{
	vec4 bufferVarVertexFragment;
};
 buffer BufferInstanceVertexFragment
{
	vec4 instance;
} bufferInstanceVertexFragment;
 void funcDeclVertexFragment(in vec3 input, out vec4 output);
 void funcVertexFragment()
{
	if (true)
	{
		gl_Position = bufferVarVertexFragment;
	}
}
