#version 330 core

in vec3 v_textureColor;

out vec4 f_Color; // fragment's final color (out to the fragment shader)

void main()
{
	f_Color = vec4(v_textureColor,1.0f);
}