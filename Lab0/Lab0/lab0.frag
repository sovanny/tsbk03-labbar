/*
 * random comment here
 * makes syntax highlight appaer
 * colors like springs sprouts
 */

#version 150

in float shade;

out vec4 out_Color;
in vec4 forFragColor;

in vec2 frag_texcoord;

uniform sampler2D exampletexture;

void main(void)
{
	//out_Color=vec4(shade,shade,shade,1.0);
	//out_Color=forFragColor;

	out_Color=texture(exampletexture,frag_texcoord);
}
