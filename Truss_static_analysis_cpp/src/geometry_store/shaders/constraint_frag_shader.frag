#version 330 core
uniform sampler2D u_Textures[2];

flat in uint v_textureType;
in vec2 v_textureCoord;
in vec3 v_textureColor;

out vec4 f_Color; // fragment's final color (out to the fragment shader)

void main()
{
	// Pin Support [0], Roller Support [1]
	vec4 texColor = texture(u_Textures[v_textureType],v_textureCoord);
	
	f_Color = vec4(v_textureColor,1.0f)*texColor;
}