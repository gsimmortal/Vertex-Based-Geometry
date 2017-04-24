#version 330

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform vec4 funkyMathValues;
uniform sampler2D hmTexture;
uniform float inTime;
uniform float inSize;

layout (location=0) in vec4 vertexPos;
layout (location=1) in vec4 vertexColour;
//layout (location=2) in vec3 vertexOffset;

vec4 calcOffset(vec4 pos);

out vec4 colour;
out vec4 newPos;
out float colProfile;
out float animate;

void main(void) {

	vec4 newVertexPosition = calcOffset(vertexPos);

	colour = vertexColour;
	newPos = newVertexPosition;
	colProfile = funkyMathValues.y;
	animate = funkyMathValues.z;

	// Hint: Remember the order the matrices are multiplied in!
	gl_Position = projectionMatrix * viewMatrix * newVertexPosition;
}

vec4 calcOffset(vec4 pos)
{
	vec4 tempHolder = pos;
	//ripple mesh deformer values
	const float amplitude = 0.5;
	const float frequency = 0.5;
	const float PI = 3.14159265;

	if (funkyMathValues.x == 0.0)//keep original y co-ord
	{
		tempHolder.y = pos.y;
	}
	else if (funkyMathValues.x == 1.0)//apply sin function to y
	{
		tempHolder.y = sin(pos.x);
	}
	else if (funkyMathValues.x == 2.0)//apply sin function with animation
	{
		tempHolder.y = sin(pos.x + funkyMathValues.z);
	}
	else if (funkyMathValues.x == 3.0)//apply user defined smoothness to sin wave
	{
		tempHolder.y = sin(funkyMathValues.w * (pos.x + funkyMathValues.z)) * sin(funkyMathValues.w * (pos.z + funkyMathValues.z));
	}
	else if (funkyMathValues.x == 4.0)//apply terrain generation function?????
	{
		//generate vec3 from point for working with length
		vec3 convPos = vec3(tempHolder.x,tempHolder.y,tempHolder.z);
		float distance = length(convPos);

		tempHolder.y = amplitude*sin(-PI*distance*frequency+inTime);
	}
	else if(funkyMathValues.x == 5.0)//Ripple Mesh Deformer Implementation
	{
		//generate vec3 from point for working with length
		vec3 convPos = vec3(tempHolder.x,tempHolder.y,tempHolder.z);
		float distance = length(convPos);

		float tempX = (inSize/2) * cos(tempHolder.x + (inSize/2)) * sin(tempHolder.z + (inSize/2));
		float tempY = (inSize/2) * cos(tempHolder.z + (inSize/2));
		float tempZ = (inSize/2) * sin(tempHolder.x + (inSize/2)) * sin(tempHolder.z + (inSize/2));

		tempHolder.x = tempX*amplitude*sin(-PI*distance*frequency+inTime);
		tempHolder.y = tempY*amplitude*sin(-PI*distance*frequency+inTime);
		tempHolder.z = tempZ*amplitude*sin(-PI*distance*frequency+inTime);
	}
	else if(funkyMathValues.x == 6.0)//terrain generation via greyscale image?
	{
		//create a temporary uv value depending on xy co-ords
		vec2 tempUV = vec2(((tempHolder.x + (inSize/2))/inSize),1.0-((tempHolder.z + (inSize/2))/inSize));
		float tempY = texture(hmTexture,tempUV).r;
		
		tempHolder.y = tempY;
	}

return tempHolder;
}