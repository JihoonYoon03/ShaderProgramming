#version 330

layout(location=0) out vec4 FragColor;

in vec2 v_TPos;

uniform float u_Time;

uniform vec4 u_DropInfo[1000];  // (x, y, sT, lT)

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

void AIFractalJulia()
{
    // 1. 좌표계 설정: v_TPos(0~1)를 NDC 범위(-1.5~1.5) 정도로 확장 및 중심 이동
    vec2 z = (v_TPos * 2.0 - 1.0) * 1.5;
    
    // 2. Julia Set의 모양을 결정하는 상수 C
    // 시간에 따라 변화를 주어 문양이 계속 움직이게 만듭니다.
    vec2 c = vec2(sin(u_Time * 0.5) * 0.4 - 0.1, cos(u_Time * 0.3) * 0.4 + 0.1);
    
    float iter = 0.0;
    const float maxIter = 64.0; // 반복 횟수가 높을수록 정교해집니다.
    
    // 3. 프랙탈 반복 계산: z = z^2 + c
    for(float i = 0.0; i < maxIter; i++) {
        // 복소수 제곱 연산: (x+yi)^2 = x^2 - y^2 + 2xyi
        float x = (z.x * z.x - z.y * z.y) + c.x;
        float y = (2.0 * z.x * z.y) + c.y;
        
        z = vec2(x, y);
        
        // 발산 체크 (거리가 2를 넘으면 프랙탈 집합에서 벗어남)
        if(length(z) > 2.0) break;
        iter++;
    }
    
    // 4. 탈출 속도(iter)에 따른 색상 결정
    if(iter == maxIter) {
        // 집합 내부: 어두운 색
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        // 집합 외부: 발산 속도에 따라 화려한 색상 부여
        float f = iter / maxIter;
        vec3 color = vec3(
            sin(f * 10.0 + u_Time), 
            sin(f * 15.0 + u_Time * 1.2), 
            cos(f * 20.0)
        ) * 0.5 + 0.5;
        
        FragColor = vec4(color, 1.0);
    }
}

void AIContinuousInfiniteZoom()
{
    // 1. 좌표 정규화 (-1.0 ~ 1.0)
    vec2 uv = v_TPos * 2.0 - 1.0;
    
    vec3 finalColor = vec3(0.0);
    float time = u_Time * 0.5; // 전체적인 속도 조절
    
    // 2. 다중 레이어 블렌딩을 하나의 루프 내에서 처리
    // 레이어를 3개 겹쳐야 앞 레이어가 사라질 때 뒷 레이어가 나타나며 흐름이 끊기지 않습니다.
    for(int i = 0; i < 3; i++) {
        float layerOffset = float(i);
        float f = fract(time + layerOffset / 3.0); // 0.0 ~ 1.0 반복되는 층별 시간
        
        // 지수적 줌: 배율이 1에서 8까지 급격히 커지도록 설정
        float zoom = pow(8.0, 1.0 - f);
        vec2 uvLayer = uv * zoom;
        
        // 3. 기하학적 형태 생성 (반복 대칭 및 회전)
        float angle = time * 0.2 + layerOffset;
        float s = sin(angle), c = cos(angle);
        mat2 rot = mat2(c, -s, s, c);
        
        for(int j = 0; j < 4; j++) {
            uvLayer = abs(uvLayer) - 0.5;
            uvLayer *= rot;
            uvLayer *= 1.2;
        }
        
        // 4. 선명도 및 투명도 계산
        // fract()의 특성을 이용해 레이어가 생성될 때(f=0)와 사라질 때(f=1) 부드럽게 페이드 처리
        float opacity = sin(f * c_PI); 
        float dist = length(uvLayer);
        float line = 0.01 / abs(sin(dist * 3.0 + time));
        
        // 5. 색상 조합 (레이어마다 색상을 다르게 순환)
        vec3 col = 0.5 + 0.5 * cos(time + vec3(0, 2, 4) + layerOffset);
        finalColor += col * line * opacity;
    }

    FragColor = vec4(finalColor, 1.0);
}

void RainDrop()
{
    float accum = 0;

    for (int i = 0; i < 1000; ++i) {
        float lTime = u_DropInfo[i].w;
        float sTime = u_DropInfo[i].z;
        float newTime = u_Time - sTime;
    
        if (newTime > 0) {
            newTime = fract(newTime / lTime);
            float oneMinus = 1 - newTime;
            float t = newTime * lTime;

	        vec2 center = u_DropInfo[i].xy;
	        vec2 currPos = v_TPos.xy;
	        float d = distance(center, currPos);

            float range = t / 10;
            float fade = 50 * clamp(range - d, 0, 1);

	        float value = pow(abs(sin(d * c_PI * 4 * 10 - t * 15)), 16);

	        accum += value * fade * oneMinus;
        }
        else {
        }
    }
    FragColor = vec4(accum);
}

void main()
{
	//CircleSin();
	//AIFractalJulia();
    //RainDrop();
    AIRainDrop();
}