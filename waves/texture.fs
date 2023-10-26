#version 430 core
precision mediump float;

out vec4 FragColor;

in vec2 TexCoord;

// texture sampler
uniform sampler2D texture1;
uniform int colourMode;

uniform float max_u;
uniform float max_ut;
uniform float max_F;

uniform float saturation;

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

// Accepts x in range +/-1
vec4 blue_to_red(float x)
{
		if (x > 0) {
			return vec4(0.f, 0.f, x, 1.f);
		}
		else {
			return vec4(-x, 0.f, 0.f, 1.f);
		}
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
	float u = texture(texture1, TexCoord).x;
	float ut = texture(texture1, TexCoord).y;
	float F = texture(texture1, TexCoord).z;

	float mm_u = u / max_u;
	float mm_ut = ut / max_ut;
	float mm_F = F / max_F;

	if (saturation > 0) {
		mm_u  = tanh(saturation * mm_u);
		mm_ut = tanh(saturation * mm_ut);
		mm_F  = tanh(saturation * mm_F);
	}


	if (colourMode == 0) {
		FragColor = blue_to_red(mm_F);
	} else if (colourMode == 1) {
		FragColor = blue_to_red(mm_u);
	} else if (colourMode == 2) {
		FragColor = blue_to_red(mm_ut);
	} else if (colourMode == 3) {
		vec3 RGB;
		if (mm_u > 0) {
			RGB = hsv_to_rgb(vec3(0.f, mm_u, 0.99f));
		} else {
			RGB = hsv_to_rgb(vec3(250.f, -mm_u, 0.99f));
		}
		FragColor = vec4(RGB, 1.f);
	} else if (colourMode == 4) {
		FragColor = leopard(mm_u);
	} else if (colourMode == 5) {
		FragColor = vec4(u, ut, ut, 1.0f);	
	} else {
		FragColor = constant_sum_colour(mm_u);
	}
}
