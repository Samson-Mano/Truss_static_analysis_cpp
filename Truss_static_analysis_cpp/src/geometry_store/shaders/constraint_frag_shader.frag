#version 330 core
uniform sampler2D u_Textures[2];

flat in uint v_textureType;
in vec2 v_textureCoord;
in vec3 v_textureColor;

out vec4 f_Color; // fragment's final color (out to the fragment shader)

void main()
{
	vec4 texColor;
	
	if(v_textureType == uint(0))
	{	
		// Pin Support
		texColor = texture(u_Textures[0],v_textureCoord);
	}
	else if(v_textureType == uint(1))
	{
		// Roller Support
		texColor = texture(u_Textures[1],v_textureCoord);
	}

	f_Color = vec4(v_textureColor,1.0f)*texColor;
}