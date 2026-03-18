#version 330

uniform float u_Time;

in vec3 a_Position;
in float a_Mass;
in vec2 a_Vel;

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

void main()
{
    Falling();
}