#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 fColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    fColor = aColor;
}

/*
#version 330 core

uniform mat4 modelMatrix;
uniform mat4 rotationMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColor;

out vec3 v_Color;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix = scalingMatrix * modelMatrix;
	mat4 rotatedModelMatrix = rotationMatrix * scaledModelMatrix; 
	mat4 translatedModelMatrix =  rotatedModelMatrix * panTranslation; 

	// apply Translation to the final position 
	vec4 finalPosition = rotatedModelMatrix * vec4(position,1.0f) * panTranslation;

	v_Color = vertexColor;
	gl_Position = finalPosition;
}
*/