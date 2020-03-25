#version 430
layout(local_size_x = 1, local_size_y = 1) in;

layout(location = 0, binding = 0, rgba32f) uniform readonly  image2D oldConc;
layout(location = 1, binding = 1, rgba32f) uniform writeonly image2D newConc;

// 
//       py0         y    
//   px0 p11 px2     |  x - >  
//       py2         V 
//

float dx    = 1;
float dt    = 0.0005;
float Da    = 1;
float Db    = 100;
float alpha = -0.005;
float beta  = 10;

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
    ivec2 p11Coords = ivec2(gl_GlobalInvocationID.xy);
    ivec2 py0Coords = p11Coords + ivec2( 0, -1);
    ivec2 py2Coords = p11Coords + ivec2( 0,  1);
    ivec2 px0Coords = p11Coords + ivec2(-1,  0);
    ivec2 px2Coords = p11Coords + ivec2( 1,  0);

    vec4 p11 = imageLoad(oldConc, p11Coords).rgba;
    vec4 py0 = p11;
    vec4 py2 = p11;
    vec4 px0 = p11;
    vec4 px2 = p11;
    if (p11Coords.y > 0) { 
        py0 = imageLoad(oldConc, py0Coords).rgba;
    }
    if (p11Coords.y < imgSize.y - 1)
    {
        py2 = imageLoad(oldConc, py2Coords).rgba;
    }
    if (p11Coords.x > 0)
    {
        px0 = imageLoad(oldConc, px0Coords).rgba;
    }
    if (p11Coords.x < imgSize.x - 1)
    {
        px2 = imageLoad(oldConc, px2Coords).rgba;
    }

    vec4 L = (px2 + px0 + py2 + py0 - 4 * p11) / (dx * dx);

    p11.x = p11.x + dt * ((Da * L.x) + Ra(p11.x, p11.y));
    p11.y = p11.y + dt * ((Db * L.y) + Rb(p11.x, p11.y));

    // output to a specific pixel in the image
    imageStore(newConc, p11Coords, p11);
}
