#version 430
layout(local_size_x = 32, local_size_y = 32) in;

layout(location = 0, binding = 0, rgba32f) uniform readonly  image2D oldConc;
layout(location = 1, binding = 1, rgba32f) uniform writeonly image2D newConc;

layout(location = 2) uniform float dx;
layout(location = 3) uniform float dt;
layout(location = 4) uniform float Da;
layout(location = 5) uniform float Db;
layout(location = 6) uniform float alpha;
layout(location = 7) uniform float beta;

// 
//       py0         y    
//   px0 p11 px2     |  x - >  
//       py2         V 
//

float Ra (float a, float b)
{
    return a - a*a*a - b + alpha;
}

float Rb(float a, float b)
{
    return beta * (a - b);
}

void main() {
    ivec2 imgSize = imageSize(oldConc);
    // get index in global work group i.e x,y position
    ivec2 py0Coords, py2Coords, px0Coords, px2Coords;
    ivec2 p11Coords = ivec2(gl_GlobalInvocationID.xy);
    
    if (p11Coords.y > 0) {
        py0Coords = p11Coords + ivec2( 0, -1);
    } else {
        py0Coords = ivec2(p11Coords.x, imgSize.y - 1);
    }

    if (p11Coords.y < imgSize.y - 1) {
        py2Coords = p11Coords + ivec2( 0,  1);
    } else {
        py2Coords = ivec2(p11Coords.x, 0);
    }

    if (p11Coords.x > 0) {
        px0Coords = p11Coords + ivec2(-1,  0);
    } else {
        px0Coords = ivec2(imgSize.x - 1, p11Coords.y);
    }

    if (p11Coords.x < imgSize.x - 1) {
        px2Coords = p11Coords + ivec2( 1,  0);
    } else {
        px2Coords = ivec2(0, p11Coords.y);
    }


    vec4 p11 = imageLoad(oldConc, p11Coords).rgba;
    vec4 py0 = imageLoad(oldConc, py0Coords).rgba;
    vec4 py2 = imageLoad(oldConc, py2Coords).rgba;
    vec4 px0 = imageLoad(oldConc, px0Coords).rgba;
    vec4 px2 = imageLoad(oldConc, px2Coords).rgba;

    vec4 L = (px2 + px0 + py2 + py0 - 4 * p11) / (dx * dx);

    p11.x = p11.x + dt * ((Da * L.x) + Ra(p11.x, p11.y));
    p11.y = p11.y + dt * ((Db * L.y) + Rb(p11.x, p11.y));

    // output to a specific pixel in the image
    imageStore(newConc, p11Coords, p11);
}
