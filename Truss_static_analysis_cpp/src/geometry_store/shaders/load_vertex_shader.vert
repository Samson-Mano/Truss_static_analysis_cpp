#version 330 core

uniform mat4 modelMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 nodeCenter;
layout(location = 2) in vec3 vertexColor;

out vec3 v_textureColor;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix = scalingMatrix * modelMatrix;
	
	// apply Translation to the final position 
	vec4 finalPosition = scaledModelMatrix * vec4(position,1.0f) * panTranslation;

	// apply Translation to the constraint center
	vec4 finalnodeCenter = scaledModelMatrix * vec4(nodeCenter,1.0f) * panTranslation;

	v_textureColor = vertexColor;

	// Scale the final position
	vec2 scaled_pt = vec2(finalPosition.x - finalnodeCenter.x,finalPosition.y - finalnodeCenter.y) / zoomscale;

	// Final position passed to shader
	gl_Position = vec4(finalnodeCenter.x + scaled_pt.x,finalnodeCenter.y + scaled_pt.y,0.0f,1.0f);
}