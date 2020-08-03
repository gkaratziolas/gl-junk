#version 330 core
out vec4 FragColor;

uniform float phase;
uniform float aspect_ratio;
in vec3 ourColor;

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
	float scale_factor = 5;
	float x = ourColor.x * scale_factor * aspect_ratio;
	float y = ourColor.y * scale_factor;
	float r = length(vec2(x, y));

	//float H = 180 * cos(10*x*x - phase/100) + 180 * cos(10*y*x - phase/50);
	//float H = 360 * tan(r - phase/1000) + 360 * tan(x*y - phase/1000);
	//float H = 360 * sin(1/(x*x+y*y)* phase/1000);
	float H = 360*x*y*sin(x+y + phase/10);
	float r0 = length(vec2(x, y) - vec2(-0.5, 0));
	float r1 = length(vec2(x, y) - vec2(0.5, 0));
	float r2 = length(vec2(x, y) - vec2(0, 0.5));
	float r3 = length(vec2(x, y) - vec2(0, -0.5));

	int N = 4;
	float R = 0.5;
	//float H = 0;
	//for (int i = 0; i < N; i++) {
	//	r = length(vec2(x, y) - vec2(R * cos(6.283185 * float(i) / float(N)),
	//	                             R * sin(6.283185 * float(i) / float(N))));
	//	H += 180 * cos(r * phase / 10);
	//}

	//float H = 180 * cos(r0 * phase / 10)
	//        + 180 * cos(r1 * phase / 10)
	//		+ 180 * cos(r2 * phase / 10)
	//		+ 180 * cos(r3 * phase / 10);
	//float H = 10000/(r*r) - phase;
	//float H = ourColor.x * 180;
	float S = 0.9f;//1 - length(vec2(ourColor.x, ourColor.y)) / sqrt(2);
	float L = 0.7f;
	vec3 hsl = vec3(H, S, L);

	vec3 RGB = hsl_to_rgb(hsl);

	FragColor = vec4(RGB, 1.0f);
}
