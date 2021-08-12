//Fragment Program
#version 130

uniform samplerCube fpCube;
out vec4 vColorPixel;
in vec3 vTexVary;
void main (void)
{
	vColorPixel = texture(fpCube, vTexVary);
}
