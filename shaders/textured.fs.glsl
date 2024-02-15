#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;
uniform float damageIntensity;

// Output color
layout(location = 0) out  vec4 color;

/* comments
void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
}
comments */

void main() {
    // Interpolate between the original color and red based on damage intensity
    vec3 damagedColor = mix(fcolor, vec3(1.0, 0.0, 0.0), damageIntensity);
    color = vec4(damagedColor, 1.0) * texture(sampler0, texcoord);
}
