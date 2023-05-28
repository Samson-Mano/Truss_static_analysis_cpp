#version 330 core

uniform mat4 modelMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;
uniform float deflscale;
uniform float geom_scale;

layout(location = 0) in vec2 node_quad_position;
layout(location = 1) in vec2 node_Center;
layout(location = 2) in vec2 node_defl;
layout(location = 3) in vec3 vertexColor;
layout(location = 4) in vec2 textureCoord;
layout(location = 5) in float is_defl;

out vec2 v_textureCoord;
out vec3 v_textureColor;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix = scalingMatrix * modelMatrix;
	
	// Declare variable to strore final quad and final node center
	vec4 finalquadPosition;
	vec4 finalnodeCenter;

	if(is_defl == 0)
	{
		// apply Translation to the final position 
		finalquadPosition = scaledModelMatrix * vec4(node_quad_position,0.0f,1.0f) * panTranslation;

		// apply Translation to the constraint center
		finalnodeCenter = scaledModelMatrix * vec4(node_Center,0.0f,1.0f) * panTranslation;
	}
	else
	{
		// Scale based on model
		float node_circe_radii = 0.005f;
		float defl_ratio = deflscale * (node_circe_radii/ geom_scale);

		// Scale the deflection point
		vec2 defl_quad_position = vec2(node_quad_position.x + (node_defl.x * defl_ratio), node_quad_position.y - (node_defl.y * defl_ratio));
		vec2 defl_position = vec2(node_Center.x + (node_defl.x * defl_ratio), node_Center.y - (node_defl.y * defl_ratio));

		// apply Translation to the final position 
		finalquadPosition = scaledModelMatrix * vec4(defl_quad_position,0.0f,1.0f) * panTranslation;

		// apply Translation to the constraint center
		finalnodeCenter = scaledModelMatrix * vec4(defl_position,0.0f,1.0f) * panTranslation;
	}
	
	v_textureCoord = textureCoord;
	v_textureColor = vertexColor;

	// Scale the final position
	vec2 scaled_pt = vec2(finalquadPosition.x - finalnodeCenter.x,finalquadPosition.y - finalnodeCenter.y) / zoomscale;

	// Final position passed to shader
	gl_Position = vec4(finalnodeCenter.x + scaled_pt.x,finalnodeCenter.y + scaled_pt.y,0.0f,1.0f);
}