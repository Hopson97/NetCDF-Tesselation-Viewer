#version 460 core

in float te_height;

out vec4 out_colour;

uniform float height_scale;
uniform float height_offset;

void main()
{
    // Normalize height based on provided scale and offset
    float h = (te_height + height_offset) / height_scale;

    // Colors: Blue for negative heights, Green for positive heights
    if (h < 0.0)
    {
        // For negative height, make the color progressively darker blue
        out_colour = vec4(0.0, 0.0, 1.0 + h, 1.0); // Blue shades, darker as h approaches -1
    }
    else
    {
        // For positive height, make the color progressively greener
        out_colour = vec4(0.0, 1.0 - h * 0.5, 0.0, 1.0); // Green shades
    }
}
