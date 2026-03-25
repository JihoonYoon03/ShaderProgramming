#version 330

uniform float u_Time;

in vec3 a_Position;
in float a_Mass;
in vec2 a_Vel;
in float a_RV0;
in float a_RV1;
in float a_RV2;

out float v_Grey;

const float c_PI = 3.141592;
const float c_G = -9.8;

// AI
// --- Pseudo Random Generator 함수 추가 ---
// vec2를 시드로 받아 0.0 ~ 1.0 사이의 float 난수를 생성합니다.
float Hash(vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

void Sin0()
{
    float startTime = a_RV0 * 2.0f;
    float newTime = u_Time - startTime;

    if (newTime > 0) {
        float lifeScale = 2.0f;
        float lifeTime = Hash(a_Vel);
        float t = mod(newTime * 2, lifeTime * lifeScale);

        float amplitude = (1 - t) * (0.5f - a_RV1);
        float frequency = a_RV2;
        float scale = (1.0f - t) * Hash(a_Vel);

        vec4 newPosition;

        newPosition.x = a_Position.x * scale + t * 2 - 1.0f;
        newPosition.y = a_Position.y * scale + sin(c_PI * 2 * t * frequency) * amplitude;
        newPosition.z = 0.0;
        newPosition.w = 1.0;

        gl_Position = newPosition;
        v_Grey = 1-t;
    }
    else {
        gl_Position = vec4(-100000, 10000, 100, 0);
        v_Grey = 0;
    }
}

void Sin1()
{
    float t = u_Time;
    vec4 newPosition;

    newPosition.x = a_Position.x + u_Time;
    newPosition.y = a_Position.y + sin(c_PI * 2 * t) * 0.5f;
    newPosition.z = 0.0;
    newPosition.w = 1.0;

    gl_Position = newPosition;
}

void Sin2()
{
    float t = u_Time;
    float freq = 1.f;
    vec4 newPosition;

    newPosition.x = a_Position.x + u_Time - 1.f;
    newPosition.y = a_Position.y + sin(c_PI * freq * t) * 0.5f;
    newPosition.z = 0.0;
    newPosition.w = 1.0;

    gl_Position = newPosition;
}

void Circle()
{
    float t = u_Time;
    float freq = 1.f;
    vec4 newPosition;

    newPosition.x = a_Position.x + cos(c_PI * freq * t);
    newPosition.y = a_Position.y + sin(c_PI * freq * t);
    newPosition.z = 0.0;
    newPosition.w = 1.0;

    gl_Position = newPosition;
}

void AI_motion()
{
    float PI = 3.141592;
    float t = u_Time * 2.0 * PI;

    // 부드러운 좌우 이동 (ease in/out)
    float moveX = sin(t) * 0.5;

    // 위아래 bounce
    float moveY = abs(sin(t * 2.0)) * 0.2;

    // squash & stretch (사이즈 과장)
    float scaleX = 1.0 + 0.4 * abs(sin(t));
    float scaleY = 1.0 - 0.3 * abs(sin(t));

    vec3 pos = a_Position;

    // 스케일 적용
    pos.x *= scaleX;
    pos.y *= scaleY;

    // 이동 적용
    pos.x += moveX;
    pos.y += moveY;

    gl_Position = vec4(pos, 1.0);
}

void Falling()
{
    float startTime = a_RV1;
    float newTime = u_Time - startTime;

    if (newTime > 0) {
        float lifeTime = a_RV2;
        float t = mod(newTime, a_RV2);
        float tt = t * t;
        float vx, vy;
        vx = a_Vel.x;
        vy = a_Vel.y;

        vec4 newPos;
        newPos.x = a_Position.x * Hash(a_Vel) + vx * t;
        newPos.y = a_Position.y * Hash(a_Vel) + vy * t + 0.5 * c_G * tt;
        newPos.z = 0;
        newPos.w = 1;
        gl_Position = newPos;
    }
    else {
        gl_Position = vec4(-1000, 0, 0, 0);
    }
}

void Falling_circle()
{
    float startTime = a_RV1;
    float newTime = u_Time - startTime;

    if (newTime > 0) {
        
        float lifeScale = 2.0f;
        float lifeTime = 0.5f * a_RV2 * lifeScale;
        // float t = lifeTime * fract(newTime / lifeTime); // fract() -> 0.0 ~ 1.0, lifeTime 곱해 0.0 ~ lifeTime 구간 반복
        float t = mod(newTime, lifeTime);   // 위와 동일
        float tt = t * t;
        float vx, vy;
        float sx, sy;
        vx = a_Vel.x / 3;
        vy = a_Vel.y / 3;

        vec4 newPos;
        sx = a_Position.x * (lifeTime - mod(newTime, lifeTime)) + sin(c_PI * 2 * a_RV0);
        sy = a_Position.y * (lifeTime - mod(newTime, lifeTime)) + cos(c_PI * 2 * a_RV0);

        newPos.x = sx + vx * t * a_RV0;
        newPos.y = sy + vy * t + 0.5 * c_G * tt;
        newPos.z = 0;
        newPos.w = 1;

        gl_Position = newPos;
    }
    else {
        gl_Position = vec4(-1000, 0, 0, 0);
    }
}

void main()
{
    Sin0();
}