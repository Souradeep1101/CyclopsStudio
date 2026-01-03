#shader vertex
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;

out vec2 v_TexCoord;
uniform mat4 u_MVP; // Model View Projection Matrix

void main()
{
   gl_Position = u_MVP * vec4(position, 1.0);
   v_TexCoord = texCoord;
}

#shader fragment
#version 330 core
layout(location = 0) out vec4 color;
in vec2 v_TexCoord;

uniform vec4 u_Color; // Brush Color

void main()
{
    // Simple signed distance field (SDF) for a soft circle
    // distance from center (0.5, 0.5)
    float dist = distance(v_TexCoord, vec2(0.5));
    
    // Create a smooth edge (0.0 at radius 0.5, 1.0 at center)
    float delta = 0.05; // Softness
    float alpha = 1.0 - smoothstep(0.5 - delta, 0.5, dist);
    
    color = vec4(u_Color.rgb, u_Color.a * alpha);
}