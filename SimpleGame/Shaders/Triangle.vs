#version 330

uniform float u_Time;

in vec3 a_Position;
in float a_Mass;
in vec2 a_Vel;
in float a_RV0;
in float a_RV1;

const float c_PI = 3.141592;
const float c_G = -9.8;

void Sin1()
{
    float t = u_Time;
    vec4 newPosition;

    newPosition.x = a_Position.x + u_Time;
    newPosition.y = a_Position.y + sin(3.141592 * 2 * t) * 0.5f;
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
    newPosition.y = a_Position.y + sin(3.141592 * freq * t) * 0.5f;
    newPosition.z = 0.0;
    newPosition.w = 1.0;

    gl_Position = newPosition;
}

void Circle()
{
    float t = u_Time;
    float freq = 1.f;
    vec4 newPosition;

    newPosition.x = a_Position.x + cos(3.141592 * freq * t);
    newPosition.y = a_Position.y + sin(3.141592 * freq * t);
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
    float t = mod(u_Time, 1.0);
    float tt = t * t;
    float vx, vy;
    vx = a_Vel.x;
    vy = a_Vel.y;

    vec4 newPos;
    newPos.x = a_Position.x + vx * t;
    newPos.y = a_Position.y + vy * t + 0.5 * c_G * tt;
    newPos.z = 0;
    newPos.w = 1;

    gl_Position = newPos;
}

// AI
// --- Pseudo Random Generator 함수 추가 ---
// vec2를 시드로 받아 0.0 ~ 1.0 사이의 float 난수를 생성합니다.
float Hash(vec2 seed)
{
    return fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453123);
}

void Falling_circle()
{
    float startTime = a_RV1;
    float newTime = u_Time - startTime;

    if (newTime > 0)
    {
        float t = mod(newTime, 1.0);
        float tt = t * t;
        float vx, vy;
        float sx, sy;
        vx = a_Vel.x / 3;
        vy = a_Vel.y / 3;

        vec4 newPos;
        sx = a_Position.x * Hash(a_Vel) + sin(c_PI * 2 * a_RV0);
        sy = a_Position.y * Hash(a_Vel) + cos(c_PI * 2 * a_RV0);

        newPos.x = sx + vx * t * a_RV0;
        newPos.y = sy + vy * t + 0.5 * c_G * tt;
        newPos.z = 0;
        newPos.w = 1;

        gl_Position = newPos;
    }
    else
    {
        gl_Position = vec4(-1000, 0, 0, 0);
    }
}

void main()
{
    Falling_circle();
}