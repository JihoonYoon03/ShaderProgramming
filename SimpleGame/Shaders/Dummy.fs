#version 330

layout(location=0) out vec4 FragColor;

uniform sampler2D u_AsdfTex;

in float v_Grey;
in vec2 v_Tex;

const float c_PI = 3.141592;

mat2 rotate2D(float radian)
{
	return mat2(cos(radian), -sin(radian), sin(radian), cos(radian));
}

float LinePattern()
{
	float lineCountH = 20;
	float lineCountV = 20;
	float lineWidth = 1;
	lineCountH = lineCountH / 2;
	lineCountV = lineCountV / 2;
	lineWidth = 100 / lineWidth;
	float per = c_PI / 2;

	float greyX = pow(abs(sin(v_Tex.x * c_PI * 2 * lineCountV - per)), lineWidth);
	float greyY = pow(abs(sin(v_Tex.y * c_PI * 2 * lineCountH - per)), lineWidth);
	
	// 위와 동일
	// float greyX = pow(abs(cos(v_TPos.x * c_PI * 2 * lineCountH)), lineWidth);
	// float greyY = pow(abs(cos(v_TPos.y * c_PI * 2 * lineCountV)), lineWidth);

	return greyX + greyY;
}

void Frag()
{
	vec2 newTex = rotate2D(-c_PI / 2.0) * v_Tex;

	FragColor = texture(u_AsdfTex, newTex);
}

void main()
{
	FragColor = vec4(v_Grey);
}
