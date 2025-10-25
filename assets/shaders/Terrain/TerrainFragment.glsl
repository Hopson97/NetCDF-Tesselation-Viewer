#version 460 core

in float te_height;

out vec4 out_colour;

uniform float height_scale;
uniform float height_offset;


void main()
{
	float h = (te_height + height_offset) / height_scale;
	out_colour = vec4(h, h, h, 1.0);
}