#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 payload;

// barycentric coordinates of the hit point (u, v) — provided by the hardware
hitAttributeEXT vec2 attribs;

void main()
{
    // Reconstruct all three barycentric weights and use them as an RGB colour
    const vec3 barycentrics = vec3(1.0 - attribs.x - attribs.y, attribs.x, attribs.y);
    payload                 = barycentrics;
    // payload = vec3(1.0, 0.0, 0.0);
}
