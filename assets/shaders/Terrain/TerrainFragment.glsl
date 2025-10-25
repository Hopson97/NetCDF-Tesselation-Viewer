#version 460 core

in float te_height;

out vec4 out_colour;

void main()
{
	float h = (te_height + 16) / 64.0;
	out_colour = vec4(h, h, h, 1.0);
}