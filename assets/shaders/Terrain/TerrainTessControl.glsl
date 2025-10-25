#version 460 core


layout (vertices = 4) out;

in vec2 vs_texture_coord[];

out vec2 tc_texture_coords[];

uniform mat4 view_matrix;
uniform mat4 model_matrix;
uniform float min_distance;
uniform float max_distance;
uniform int min_tess_level;
uniform int max_tess_level;

void main()
{

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tc_texture_coords[gl_InvocationID] = vs_texture_coord[gl_InvocationID];

    if (gl_InvocationID == 0)
    {
        // Transform each vertex into eye space
        vec4 eyeSpacePos00 = view_matrix * model_matrix * gl_in[0].gl_Position;
        vec4 eyeSpacePos01 = view_matrix * model_matrix * gl_in[1].gl_Position;
        vec4 eyeSpacePos10 = view_matrix * model_matrix * gl_in[2].gl_Position;
        vec4 eyeSpacePos11 = view_matrix * model_matrix * gl_in[3].gl_Position;

        // Distance from camera scaled between 0 and 1
        float distance00 = clamp((abs(eyeSpacePos00.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        float distance01 = clamp((abs(eyeSpacePos01.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        float distance10 = clamp((abs(eyeSpacePos10.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);
        float distance11 = clamp((abs(eyeSpacePos11.z) - min_distance) / (max_distance - min_distance), 0.0, 1.0);

        // Interpolate edge tessellation level based on closer vertex
        float tessLevel0 = mix(max_tess_level, min_tess_level, min(distance10, distance00) );
        float tessLevel1 = mix(max_tess_level, min_tess_level, min(distance00, distance01) );
        float tessLevel2 = mix(max_tess_level, min_tess_level, min(distance01, distance11) );
        float tessLevel3 = mix(max_tess_level, min_tess_level, min(distance11, distance10) );

        // Set the corresponding outer edge tessellation levels
        gl_TessLevelOuter[0] = tessLevel0;
        gl_TessLevelOuter[1] = tessLevel1;
        gl_TessLevelOuter[2] = tessLevel2;
        gl_TessLevelOuter[3] = tessLevel3;

        // Set the inner tessellation levels to the max of the two parallel edges
        gl_TessLevelInner[0] = max(tessLevel1, tessLevel3);
        gl_TessLevelInner[1] = max(tessLevel0, tessLevel2);
    }
}