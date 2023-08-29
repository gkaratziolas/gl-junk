#version 430

layout(local_size_x = 32, local_size_y = 32) in;

layout(location = 0, binding = 0, rgba32f) uniform readonly image2D  oldWorld;
layout(location = 1, binding = 1, rgba32f) uniform writeonly image2D newWorld;

layout(location = 2) uniform float dx;
layout(location = 3) uniform float dt;
layout(location = 4) uniform float c;

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
 *      py0         y
 * px0  p11  px2    |   x - >
 *      py2         V
 */

void main() {
    ivec2 imgSize = imageSize(oldWorld);
    // get index in global work group i.e x,y position
    ivec2 py0Coords, py2Coords, px0Coords, px2Coords;
    ivec2 p11Coords = ivec2(gl_GlobalInvocationID.xy);

    vec4 p11 = imageLoad(oldWorld, p11Coords).rgba;

    vec4 py0 = vec4(0.0, 0.0, 0.0, 0.0);
    if (p11Coords.y > 0) {
        py0 = imageLoad(oldWorld, p11Coords + ivec2(0, -1)).rgba;
        //py0 = p11;
    }

    vec4 py2 = vec4(0.0, 0.0, 0.0, 0.0);
    if (p11Coords.y < imgSize.y - 1) {
        py2 = imageLoad(oldWorld, p11Coords + ivec2(0, 1)).rgba;
        //py2 = p11;
    }

    vec4 px0 = vec4(0.0, 0.0, 0.0, 0.0);
    if (p11Coords.x > 0) {
        px0 = imageLoad(oldWorld, p11Coords + ivec2(-1, 0)).rgba;
        //px0 = p11;
    }

    vec4 px2 = vec4(0.0, 0.0, 0.0, 0.0);
    if (p11Coords.x < imgSize.x - 1) {
        px2 = imageLoad(oldWorld, p11Coords + ivec2(1, 0)).rgba;
        //px2 = p11;
    }
    
    float Lu = (px2.x + px0.x + py2.x + py0.x - 4 * p11.x) / (dx * dx);

    // Update du/dt using the laplacian and force
    float new_du_dt = p11.y + dt * (c * c * Lu - p11.z);

    // Update u using the new du/dt
    float new_u = p11.x + dt * new_du_dt;

    // Copy force through
    float new_f = p11.z;
    float new_boundary = p11.w;

    vec4 outValue = vec4(new_u, new_du_dt, new_f, new_boundary);

    if (new_boundary > 0.5)
    {
        outValue = vec4(0.0, 0.0, 0.0, new_boundary);
    }

    imageStore(newWorld, p11Coords, outValue);
}
