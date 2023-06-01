#version 330 core

uniform mat4 modelMatrix;
uniform mat4 panTranslation;
uniform float zoomscale;
uniform float deflscale;
uniform float geom_scale;
uniform int soln_option;

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 node_defl;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in float is_defl;

out vec3 v_Color;

void main()
{
	// apply zoom scaling and Rotation to model matrix
	mat4 scalingMatrix = mat4(1.0)*zoomscale;
	scalingMatrix[3][3] = 1.0f;
	mat4 scaledModelMatrix = scalingMatrix * modelMatrix;

	vec4 finalPosition;

	if(is_defl == 0)
	{
		// apply Translation to the final position 
		finalPosition = scaledModelMatrix * vec4(position,0.0f,1.0f) * panTranslation;
	}
	else
	{
		// Scale the deflection
		float node_circe_radii = 0.005f;
		float defl_ratio = deflscale * (node_circe_radii/ geom_scale);
	
		vec2 defl_position = vec2(position.x + (node_defl.x * defl_ratio), position.y - (node_defl.y * defl_ratio));

		finalPosition = scaledModelMatrix * vec4(defl_position,0.0f,1.0f) * panTranslation;
	}
	
	v_Color = vertexColor;
	gl_Position = finalPosition;
}