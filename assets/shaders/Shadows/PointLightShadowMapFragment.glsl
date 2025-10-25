#version 460

in vec4 fragment_position;

uniform vec3 light_position;
uniform float far_plane;

void main()
{
    // Distance between the fragment and the light source
    float light_distance = length(fragment_position.xyz - light_position);
    
    // Map to [0:1] range by dividing by far_plane
    light_distance = light_distance / far_plane;
    
    // Write this as a modified depth
    gl_FragDepth = light_distance;
}  