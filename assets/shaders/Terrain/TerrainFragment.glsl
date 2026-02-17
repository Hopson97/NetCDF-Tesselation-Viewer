#version 460 core

in float te_height;
in vec3 te_normal;
out vec4 out_colour;

uniform float height_scale;

uniform float min_height;
uniform float max_height; 

// Colour for underwater
const vec3 SHALLOW_BLUE = vec3(0.18, 0.91, 0.91);
const vec3 DEEP_BLUE = vec3(0.0, 0.2, 1.0);
const vec3 PURPLE = vec3(0.5, 0.0, 0.5);

// Colour for land
const vec3 land = vec3(0.1, 0.4, 0.1);
const vec3 rock  = vec3(0.8, 0.7, 0.4);
const vec3 snow   = vec3(1.0, 1.0, 1.0);

void main()
{
    float height = te_height / height_scale;

    // Calculate the terrain colour using a colour ramp
    vec3 color;
    if (height < 0.0) 
    {
        float underwater_height = clamp(height / min_height, 0.0, 1.0); 
        if (underwater_height < 0.5) 
        {
            color = mix(SHALLOW_BLUE, DEEP_BLUE, underwater_height * 2.0);
        } 
        else 
        {
            color = mix(DEEP_BLUE, PURPLE, (underwater_height - 0.5) * 2.0);
        }
    } 
    else 
    {
        float land_heght = clamp(height / max_height, 0.0, 1.0);
        if (land_heght < 0.5) 
        {
            color = mix(land, rock, land_heght * 2.0);
        } 
        else 
        {
            color = mix(rock, snow, (land_heght - 0.5) * 2.0);
        }
    }

    // Simple lighting
    vec3 light_dir = normalize(vec3(0.5, 1.0, 0.2)); 
    float diff = max(dot(te_normal, light_dir), 0.0);
    vec3 ambient = 0.99 * color;
    vec3 diffuse = diff * color;
    out_colour = vec4(ambient + diffuse, 1.0);

}