#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;

void main()
{
	vec4 TexColor = texture(texture1, TexCoord);
	FragColor = vec4(1.0f - TexColor.x, 0.0f, TexColor.x, 1.0f);
}
