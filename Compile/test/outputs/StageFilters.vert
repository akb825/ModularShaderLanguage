struct StructAll
{
	vec4 value;
};
struct InstanceStructAll
{
	vec4 value;
} structInstanceAll;
 struct StructVertex
{
	vec4 value;
};
 struct InstanceStructVertex
{
	vec4 value;
} structInstanceVertex;
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
uniform UniformBlockVertex
{
	vec4 uniformBlockVarVertex;
};
 uniform UniformBlockInstanceVertex
{
	vec4 instance;
} uniformInstanceVertex;
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
int globalVarVertex;
in vec2 inVarVertex;
 out vec3 outVarVertex;
uniform sampler2D unfiformVarVertex;
 in inBufferVertex
{
	vec2 inBufferVarVertex;
};
 in inBufferVertex
{
	vec2 instance;
} inInstanceVertex;
 out outBufferVertex
{
	vec2 outBufferVarVertex;
};
 out outBufferVertex
{
	vec2 instance;
} outInstanceVertex;
 out vec3 outVarVertex;
 layout (std140)
buffer BufferVertex
{
	vec4 bufferVarVertex;
};
 buffer BufferInstanceVertex
{
	vec4 instance;
} bufferInstanceVertex;
 void funcDeclVertex(in vec3 input, out vec4 output);
 void funcVertex()
{
	if (true)
	{
		gl_Position = bufferVarVertex;
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
