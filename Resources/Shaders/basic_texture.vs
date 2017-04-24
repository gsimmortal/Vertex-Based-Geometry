#version 330

uniform mat4 mvpMatrix;

layout (location=0) in vec4 vertexPos;
layout (location=3) in vec2 vertexTexCoord;

out vec2 texCoord;

void main(void) {

	mat4 M;
	M[0] = vec4(1.0);

	ivec2 a = ivec2(1, 2);
	//vec3 b = vec3(2.0, 4.0, 1.0) + a;

	texCoord = vertexTexCoord;
	gl_Position = mvpMatrix * vertexPos;
}
