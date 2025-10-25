#version 460 core

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_texture_coord;
layout(location = 2) in vec3 in_normal;

out vec2 vs_texture_coord;

void main()
{
    gl_Position = vec4(in_position, 1.0);
    vs_texture_coord = in_texture_coord;
}
	