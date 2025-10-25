#version 460

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

uniform mat4 shadow_matrices[6];

out vec4 fragment_position;

void main()
{
    // Loop each face direction of the point light
    for(int face = 0; face < 6; face++)
    {
        gl_Layer = face; 

        // For each triangle vertex
        for(int i = 0; i < 3; i++) 
        {
            fragment_position = gl_in[i].gl_Position;
            gl_Position = shadow_matrices[face] * fragment_position;
            EmitVertex();
        }    
        EndPrimitive();
    }
}