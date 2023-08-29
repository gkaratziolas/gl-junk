#version 430 core
out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;
uniform int colourMode;

vec4 linear_colour(float x)
{
	float r = 1.0f - x;
	float b = x;
	float g;
	if (x < 0.5) {
		g = 4.0f * x - 1.0f;
	} else {
		g = -4.0f * x - 3.0f;
	}
	if (g < 0.0f) {
		g = 0.0f;
	}
	return vec4(r, g, b, 1.0f);	
}

vec4 leopard(float x)
{
	float r = 0.18039f + x * 0.74117f;
	float g = 0.14901f + x * 0.52941f;
	float b = 0.06667f;

	return vec4(r, g, b, 1.0f);
}

vec4 constant_sum_colour(float x)
{
	float pi = 3.1415926f;
	
	x = 1.0f - x;
	float r = (sin(pi*x) + 1.0f) / 2.0f;
	float g = (sin(pi*(x + 2.0f/3.0f)) + 1.0f) / 2.0f;
	float b = (sin(pi*(x + 4.0f/3.0f)) + 1.0f) / 2.0f;
	return vec4(r, g, b, 1.0f);
}

void main()
{
	float intensity = texture(texture1, TexCoord).x;

	if (colourMode == 0) {
		FragColor = vec4(intensity, 0.f, 0.f, 1.f);
	} else if (colourMode == 1) {
		FragColor = vec4(0.f, intensity, 0.f, 1.f);
	} else if (colourMode == 2) {
		FragColor = vec4(0.f, 0.f, intensity, 1.f);
	} else if (colourMode == 3) {
		FragColor = linear_colour(intensity);
	} else if (colourMode == 4) {
		FragColor = leopard(intensity);
	} else {
		FragColor = constant_sum_colour(intensity);
	}
}
