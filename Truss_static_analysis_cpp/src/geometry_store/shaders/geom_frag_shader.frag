#version 330 core

in vec3 v_Color;
out vec4 f_Color; // fragment's final color (out to the fragment shader)


void main()
{
	f_Color = vec4(v_Color,1.0);
}