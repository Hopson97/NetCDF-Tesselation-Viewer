// tessellation evaluation shader
#version 460 core

layout (quads, fractional_odd_spacing, ccw) in;


layout(std430, binding = 0) readonly buffer Matrices 
{
    mat4 projection;
    mat4 view;
} matrices;

uniform mat4 model_matrix;
uniform isampler2D height_map;  
uniform float height_scale;
uniform float height_offset;

in vec2 tc_texture_coords[];

out float te_height;

void main()
{
    // get patch coordinate
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // retrieve control point texture coordinates
    vec2 t00 = tc_texture_coords[0];
    vec2 t01 = tc_texture_coords[1];
    vec2 t10 = tc_texture_coords[2];
    vec2 t11 = tc_texture_coords[3];

    // bilinearly interpolate texture coordinate across patch
    vec2 t0 = (t01 - t00) * u + t00;
    vec2 t1 = (t11 - t10) * u + t10;
    vec2 texCoord = (t1 - t0) * v + t0;

    te_height = texture(height_map, texCoord).r * height_scale - height_offset;

    // ----------------------------------------------------------------------
    // retrieve control point position coordinates
    vec4 p00 = gl_in[0].gl_Position;
    vec4 p01 = gl_in[1].gl_Position;
    vec4 p10 = gl_in[2].gl_Position;
    vec4 p11 = gl_in[3].gl_Position;

    // compute patch surface normal
    vec4 uVec = p01 - p00;
    vec4 vVec = p10 - p00;
    vec4 normal = normalize( vec4(cross(vVec.xyz, uVec.xyz), 0) );

    // bilinearly interpolate position coordinate across patch
    vec4 p0 = (p01 - p00) * u + p00;
    vec4 p1 = (p11 - p10) * u + p10;
    vec4 p = (p1 - p0) * v + p0;

    // displace point along normal
    p += normal * te_height;

    // ----------------------------------------------------------------------
    // output patch point position in clip space
    gl_Position = matrices.projection * matrices.view * model_matrix * p;
}