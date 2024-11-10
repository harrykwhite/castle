#version 430 core

in flat int v_tex_index;
in vec2 v_tex_coord;
in vec4 v_blend;

out vec4 o_frag_color;

uniform sampler2D u_textures[32];

void main()
{
    vec4 tex_color = texture(u_textures[v_tex_index], v_tex_coord);
    o_frag_color = tex_color * v_blend;
}
