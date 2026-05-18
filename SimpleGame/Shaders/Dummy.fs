#version 330

layout(location=0) out vec4 FragColor;

in float v_Depth;

void main()
{
	float grey = v_Depth;
	FragColor = vec4(grey);
}
