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

vec3 hsv_to_rgb(vec3 hsv)
{
	float H = mod(hsv.x, 360.f);
	float S = mod(hsv.y, 1.f);
	float V = mod(hsv.z, 1.f);

	float C = V * S;
	float H_prime = H / 60;
	float X = C * (1 - length(mod(H_prime, 2) - 1));
	vec3 RGB1 = vec3(0.f, 0.f, 0.f);

	int H_prime_ceil = int(H_prime + 1);
	if (H_prime_ceil == 1) {
		RGB1 = vec3(C, X, 0.f);
	} else if (H_prime_ceil == 2) {
		RGB1 = vec3(X, C, 0.f);
	} else if (H_prime_ceil == 3) {
		RGB1 = vec3(0.f, C, X);
	} else if (H_prime_ceil == 4) {
		RGB1 = vec3(0.f, X, C);
	} else if (H_prime_ceil == 5) {
		RGB1 = vec3(X, 0.f, C);
	} else if (H_prime_ceil == 6) {
		RGB1 = vec3(C, 0.f, X);
	} else {
		RGB1 = vec3(0.f, 0.f, 0.f);
	}
	float m = V - C;
	vec3 RGB = RGB1 + m;
	return RGB;
}

vec3 hsl_to_rgb(vec3 hsl)
{
	float H = mod(hsl.x, 360.f);
	float S = mod(hsl.y, 1.f);
	float L = mod(hsl.z, 1.f);

	float C = (1.f - length(2.f * L - 1.f)) * S;
	float H_prime = H / 60;
	float X = C * (1.f - length(mod(H_prime, 2.f)- 1.f));

	vec3 RGB1 = vec3(0.f, 0.f, 0.f);

	int H_prime_ceil = int(H_prime + 1);
	if (H_prime_ceil == 1) {
		RGB1 = vec3(C, X, 0.f);
	} else if (H_prime_ceil == 2) {
		RGB1 = vec3(X, C, 0.f);
	} else if (H_prime_ceil == 3) {
		RGB1 = vec3(0.f, C, X);
	} else if (H_prime_ceil == 4) {
		RGB1 = vec3(0.f, X, C);
	} else if (H_prime_ceil == 5) {
		RGB1 = vec3(X, 0.f, C);
	} else if (H_prime_ceil == 6) {
		RGB1 = vec3(C, 0.f, X);
	} else {
		RGB1 = vec3(0.f, 0.f, 0.f);
	}

	float m = L - (C/2);

	vec3 RGB = RGB1 + m;
	return RGB;
}

void main()
{
	float intensity = texture(texture1, TexCoord).x;
	float velocity  = texture(texture1, TexCoord).y;
	float force     = texture(texture1, TexCoord).z;

	if (colourMode == 0) {
		FragColor = vec4((force - int(force)), 0.f, 0.f, 1.f);
	} else if (colourMode == 1) {
		FragColor = vec4(1.f, 1.f - intensity, 1.f, 1.f);
	} else if (colourMode == 2) {
		FragColor = vec4(1.f, 1.f - intensity, 1.f, 1.f);
	} else if (colourMode == 3) {
		vec3 RGB = hsv_to_rgb(vec3(intensity * 360, .5f, 0.8f));
		FragColor = vec4(RGB, 1.f);
	} else if (colourMode == 4) {
		FragColor = leopard(intensity);
	} else if (colourMode == 5) {
		FragColor = vec4(texture(texture1, TexCoord).x, texture(texture1, TexCoord).y, 0.0f, 1.0f);	
	} else {
		FragColor = constant_sum_colour(velocity);
	}
}
