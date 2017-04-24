
#include "stdafx.h"
#include "CGTexturedQuad.h"
#include "ShaderLoader.h"
#include "TextureLoader.h"


using namespace std;
using namespace CoreStructures;


// Geometry data for textured quad (this is rendered directly as a triangle strip)

static float quadPositionArray[] = {

	0.0f, 0.0f, 0.0f, 1.0f,
	1.0f, 0.0f, 0.0f, 1.0f,
	0.0f, 1.0f, 0.0f, 1.0f,
	1.0f, 1.0f, 0.0f, 1.0f
};

static float quadTextureCoordArray[] = {

	0.0f, 1.0f,
	1.0f, 1.0f,
	0.0f, 0.0f,
	1.0f, 0.0f
};



//
// Private API
//

void CGTexturedQuad::loadShader() {

	// setup shader for textured quad
	GLSL_ERROR glsl_err = ShaderLoader::createShaderProgram(string("..\\..\\Common\\Resources\\Shaders\\basic_texture.vs"), string("..\\..\\Common\\Resources\\Shaders\\basic_texture.fs"), &quadShader);
}


void CGTexturedQuad::setupVAO() {

	// setup VAO for textured quad object
	glGenVertexArrays(1, &quadVertexArrayObj);
	glBindVertexArray(quadVertexArrayObj);


	// setup vbo for position attribute
	glGenBuffers(1, &quadVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadPositionArray), quadPositionArray, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)0);


	// setup vbo for texture coord attribute
	glGenBuffers(1, &quadTextureCoordBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadTextureCoordBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadTextureCoordArray), quadTextureCoordArray, GL_STATIC_DRAW);

	glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, 0, (const GLvoid*)0);

	// enable vertex buffers for textured quad rendering (vertex positions and colour buffers)
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(3);

	// unbind textured quad VAO
	glBindVertexArray(0);
}


CGTexturedQuad::CGTexturedQuad(const wstring& texturePath) {

	loadShader();
	setupVAO();

	// Load texture
	texture = TextureLoader::wicLoadTexture(texturePath, TextureGenProperties(GL_LINEAR, GL_LINEAR, 2.0f, GL_REPEAT, GL_REPEAT, true));
	//texture = TextureLoader::fiLoadTexture(texturePath, TextureGenProperties(GL_LINEAR, GL_LINEAR, 2.0f, GL_REPEAT, GL_REPEAT, true));
}


CGTexturedQuad::CGTexturedQuad(GLuint initTexture) {

	loadShader();
	setupVAO();

	texture = initTexture;
}


CGTexturedQuad::~CGTexturedQuad() {

	// unbind textured quad VAO
	glBindVertexArray(0);

	// unbind VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDeleteBuffers(1, &quadVertexBuffer);
	glDeleteBuffers(1, &quadTextureCoordBuffer);

	glDeleteVertexArrays(1, &quadVertexArrayObj);

	glDeleteShader(quadShader);
}


void CGTexturedQuad::render(const GUMatrix4& T) {

	static GLint mvpLocation = glGetUniformLocation(quadShader, "mvpMatrix");

	glUseProgram(quadShader);
	glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (const GLfloat*)&(T.M));

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glBindVertexArray(quadVertexArrayObj);

	// draw quad directly - no indexing needed
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	// unbind VAO for textured quad
	glBindVertexArray(0);
}


