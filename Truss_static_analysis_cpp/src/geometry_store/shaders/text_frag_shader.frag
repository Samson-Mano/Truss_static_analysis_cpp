#version 330 core
uniform float transparency;
uniform sampler2D u_Texture;

in vec3 v_textureColor;
in vec2 v_textureCoord;

out vec4 f_Color; // fragment's final color (out to the fragment shader)

void main()
{
	vec4 texColor = vec4(1.0, 1.0, 1.0, texture(u_Texture, v_textureCoord).r);
	f_Color = vec4(v_textureColor,transparency)*texColor;
}