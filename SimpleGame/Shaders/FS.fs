#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_TPos;

uniform float u_Time;

const float c_PI = 3.141592;

void Test()
{
	vec2 fNDC = v_TPos * 2 - 1.0f;
	float len = length(fNDC);

	if (len > 0.5f){
		FragColor = vec4(0, 0, 1 * len, 1);
	}
	else if (v_TPos.x > 0.5) {
		FragColor = vec4(1 - len, 1 - len, 1 - len, 1);
	}
	else {
		FragColor = vec4(0 + len, 0 + len, 0 + len, 1);
	}
}

void Pattern()
{
	// FragColor = vec4(v_TPos.x, v_TPos.y, 0, 1);

	float lineCountH = 20;
	float lineCountV = 20;
	float lineWidth = 1;
	lineCountH = lineCountH / 2;
	lineCountV = lineCountV / 2;
	lineWidth = 100 / lineWidth;
	float per = c_PI / 2;

	float greyX = pow(abs(sin(v_TPos.x * c_PI * 2 * lineCountV - per)), lineWidth);
	float greyY = pow(abs(sin(v_TPos.y * c_PI * 2 * lineCountH - per)), lineWidth);

	// 위와 동일
	// float greyX = pow(abs(cos(v_TPos.x * c_PI * 2 * lineCountH)), lineWidth);
	// float greyY = pow(abs(cos(v_TPos.y * c_PI * 2 * lineCountV)), lineWidth);

	FragColor = vec4(greyX + greyY);
}

void Circle()
{
	vec2 center = vec2(0.5f);
	vec2 currPos = v_TPos.xy;
	float d = distance(center, currPos);
	float lineWidth = 0.01f;
	float radius = 0.1f;

	if (radius - lineWidth < d && d < radius + lineWidth){
		FragColor = vec4(v_TPos.xy, 0, 1);
	}
	else {
		FragColor = vec4(0);
	}
}

void CircleSin()
{
	vec2 center = vec2(0.5f);
	vec2 currPos = v_TPos.xy;
	float d = distance(center, currPos);
	float lineCount = 16;
	float lineWidth = 2;
	lineCount = lineCount / 2;
	lineWidth = 100 / lineWidth;

	float speed = -2;

	float value = abs(sin(d * c_PI * 2 * lineCount + (u_Time * speed)));

	FragColor = vec4(pow(value, lineWidth));
}

void main()
{
	CircleSin();
}
