#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_TPos;

void main()
{
	// 주어진 정점 정보를 통해, 폴리곤 내부의 픽셀들에 대해 연산을 수행
	// 500x500 크기의 창을 꽉 채울 경우 25만개의 픽셀에 대한 연산 수행

	if (v_TPos.x > 0.66) {
		FragColor = vec4(1, 1, 1, 1);
	}
	else if (v_TPos.x > 0.33) {
		FragColor = vec4(0.5, 0.5, 0.5, 1);
	}
	else {
		FragColor = vec4(0, 0, 0, 1);
	}
}
