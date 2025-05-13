#version 330

in vec3 pos;
in vec2 vertexTextureCoords;

uniform mat4 mvp;

out vec2 fragTextureCoords;

void main()
{
	gl_Position = mvp * vec4(pos, 1.0);

	fragTextureCoords = vertexTextureCoords;
}


