#version 430

layout(local_size_x = 32, local_size_y = 32) in;

//layout(location = 0, binding = 0, rgba32f) uniform readonly image2D  oldWorld;
//layout(location = 1, binding = 1, rgba32f) uniform writeonly image2D newWorld;

layout(binding = 0, rgba32f) uniform readonly image2D oldWorld;
layout(binding = 1, rgba32f) uniform writeonly image2D newWorld;

layout(location = 2) uniform float dx;
layout(location = 3) uniform float dt;
layout(location = 4) uniform float c;
layout(location = 5) uniform int global_boundary;
layout(location = 6) uniform float t;

/*
 * World Images
 * Old World
 * [0] u        (present - n)
 * [1] du/dt    (subpresent - n-1/2)
 * [2] F        (present force - n)
 * [3] boundary (present - n)
 * 
 * New World
 * [0] u        (future - n+1)
 * [1] du/dt    (subfuture - n+1/2)
 * [2] F        (future force - n+1)
 * [3] boundary (present - n)
 */

/*
 * global_boundary one of:
 * 0 : Dirichlet 
 * 1 : Neumann
 * 2 : Toroidal world
 */

#define BOUNDARY_DIRICHLET  0
#define BOUNDARY_NEUMANN    1
#define BOUNDARY_ABSORB     2
#define BOUNDARY_TOROIDAL   3

#define SPONGE_WIDTH        100

int distance_squared(ivec2 p, ivec2 q)
{
    int Dx_2 = (p.x - q.x) * (p.x - q.x);
    int Dy_2 = (p.y - q.y) * (p.y - q.y);
    return Dx_2 + Dy_2;
}

float gaussian(ivec2 p, ivec2 centre, float A, float sigma)
{
    float dist_squared = float(distance_squared(p, centre));
    return A * exp( -(dist_squared * dx * dx) / (2 * sigma * sigma));
}

/*
 *      py0         y
 * px0  p11  px2    |   x - >
 *      py2         V
 */

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    ivec2 imgSize = imageSize(oldWorld);
    // get index in global work group i.e x,y position
    ivec2 p11Coords = ivec2(gl_GlobalInvocationID.xy);

    vec4 p11 = imageLoad(oldWorld, p11Coords).rgba;
    vec4 py0, py2, px0, px2;

    if (global_boundary == BOUNDARY_DIRICHLET)
    {
        py0 = vec4(0.0, 0.0, 0.0, 0.0);
        py2 = vec4(0.0, 0.0, 0.0, 0.0);
        px0 = vec4(0.0, 0.0, 0.0, 0.0);
        px2 = vec4(0.0, 0.0, 0.0, 0.0);
    }
    else if (global_boundary == BOUNDARY_NEUMANN)
    {
        py0 = p11;
        py2 = p11;
        px0 = p11;
        px2 = p11;
    }
    else if (global_boundary == BOUNDARY_TOROIDAL)
    {
        py0 = imageLoad(oldWorld, ivec2(p11Coords.x, imgSize.y - 1)).rgba; ;
        py2 = imageLoad(oldWorld, ivec2(p11Coords.x, 0)).rgba; ;
        px0 = imageLoad(oldWorld, ivec2(imgSize.x - 1, p11Coords.y)).rgba; ;
        px2 = imageLoad(oldWorld, ivec2(0, p11Coords.y)).rgba; ;
    } 
    else
    {
        py0 = imageLoad(oldWorld, p11Coords + ivec2(0, -1)).rgba; ;
        py2 = imageLoad(oldWorld, p11Coords + ivec2(0, 1)).rgba; ;
        px0 = imageLoad(oldWorld, p11Coords + ivec2(-1, 0)).rgba; ;
        px2 = imageLoad(oldWorld, p11Coords + ivec2(1, 0)).rgba; ;
    }

    if (p11Coords.y > 0) {
        py0 = imageLoad(oldWorld, p11Coords + ivec2(0, -1)).rgba;
    }
    if (p11Coords.y < imgSize.y - 1) {
        py2 = imageLoad(oldWorld, p11Coords + ivec2(0, 1)).rgba;
    }
    if (p11Coords.x > 0) {
        px0 = imageLoad(oldWorld, p11Coords + ivec2(-1, 0)).rgba;
    }
    if (p11Coords.x < imgSize.x - 1) {
        px2 = imageLoad(oldWorld, p11Coords + ivec2(1, 0)).rgba;
    }
    
    // Calculate the Laplacian
    float Lu = (px2.x + px0.x + py2.x + py0.x - 4 * p11.x) / (dx * dx);

    // Update du/dt using the laplacian and force
    float new_du_dt = p11.y + dt * (c * c * Lu - p11.z);

    // Update u using the new du/dt
    float new_u = p11.x + dt * new_du_dt;

    // Copy force through
    float new_f = 0;
    new_f += gaussian(p11Coords, ivec2(512, 200), 100, 10) * cos(t * 10);
    new_f += gaussian(p11Coords, ivec2(300, 300), 30, 10) * cos(t * 9);
    float attenuation = p11.w;


    //float x0, y0;
    //x0 = 512 + 300 * cos(t * 0.5);
    //y0 = 512 + 300 * sin(t * 0.5);
    //new_f += gaussian(p11Coords, ivec2(x0, y0), 300, 3) * cos(t * 30);

    //if ((p11Coords.x < SPONGE_WIDTH) || ((imgSize.x - p11Coords.x - 1) < SPONGE_WIDTH) ||
    //    (p11Coords.y < SPONGE_WIDTH) || ((imgSize.y - p11Coords.y - 1) < SPONGE_WIDTH)) {
    //    outValue = vec4(0.999 * new_u, new_du_dt, 0.0, new_boundary);
    //}


    vec4 outValue = vec4(new_u, new_du_dt, new_f, attenuation);
    if (attenuation > 0)
    {
        if (attenuation > 1)
            attenuation = 1;
        if (attenuation < 0)
            attenuation = 0;
        float k = 1 - attenuation;
        outValue = vec4(k * new_u, new_du_dt, new_f, attenuation);
    }

    imageStore(newWorld, p11Coords, outValue);
}
