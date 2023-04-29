#version 330 core

in vec4 v_Color;
out vec4 f_Color; // fragment's final color (out to the fragment shader)


void main()
{
	f_Color = v_Color;
}