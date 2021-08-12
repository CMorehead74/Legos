//Fragment Program
//floor.fp
#version 130

uniform sampler2D colorMap;
uniform sampler2D detailMap;
out vec4 vColorPixel;
in vec2 vTexCoordVary;
void main (void)
{
	vColorPixel = texture(colorMap, vTexCoordVary);

if (vColorPixel.a < 0.2f)
discard;

	vColorPixel *= texture(detailMap, vTexCoordVary);


}
