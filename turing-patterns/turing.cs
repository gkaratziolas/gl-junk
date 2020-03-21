#version 430
layout(local_size_x = 1, local_size_y = 1) in;
layout(rgba32f, binding = 0) uniform image2D img_output;

void main() {
	// get index in global work group i.e x,y position
	ivec2 pixel_coords = ivec2(gl_GlobalInvocationID.xy);
	ivec2 dims = imageSize(img_output);
	vec4 pixel = vec4(float(pixel_coords.x) / dims.x, 0.0f, float(pixel_coords.y) / dims.y, 1.0f);
	 	
	// output to a specific pixel in the image
	imageStore(img_output, pixel_coords, pixel);
}
