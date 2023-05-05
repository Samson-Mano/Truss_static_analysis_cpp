#version 330 core

uniform mat4 modelMatrix;
uniform mat4 rotationMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;
uniform float node_size;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 textureCoord;

out vec2 v_textureCoord;
out vec3 v_textureColor;

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

	v_textureCoord = textureCoord;
	v_textureColor = vertexColor;
	
	// Ad just the final position based on zoom scale
	vec2 node_origin;

	// below code is to make sure the size of the texture does not scale with the zoom operation
	// base on the texture co-ordinate find the origin (or mid) of the texture to rescale
	if (textureCoord[0] == 0)
	{
		if(textureCoord[1] ==0)
		{
			// 00 Texture coord
			node_origin = vec2(finalPosition[0] + (node_size* zoomscale), finalPosition[1] + (node_size * zoomscale));
			finalPosition = vec4(node_origin[0] - node_size, node_origin[1] - node_size, 0.0f, 1.0f);
		}
		else
		{
			// 01 Texture coord
			node_origin = vec2(finalPosition[0] + (node_size * zoomscale), finalPosition[1] - (node_size * zoomscale));
			finalPosition = vec4(node_origin[0] - node_size, node_origin[1] + node_size, 0.0f, 1.0f);
		}
	}
	else
	{
		if(textureCoord[1] ==0)
		{
			// 10 Texture coord
			node_origin = vec2(finalPosition[0] - (node_size * zoomscale), finalPosition[1] + (node_size * zoomscale));
			finalPosition = vec4(node_origin[0] + node_size, node_origin[1] - node_size, 0.0f, 1.0f);
		}
		else
		{
			// 11 Texture coord
			node_origin = vec2(finalPosition[0] - (node_size * zoomscale), finalPosition[1] - (node_size * zoomscale));
			finalPosition = vec4(node_origin[0] + node_size, node_origin[1] + node_size, 0.0f, 1.0f);
		}
	}

	// Final position passed to shader
	gl_Position = finalPosition;
}