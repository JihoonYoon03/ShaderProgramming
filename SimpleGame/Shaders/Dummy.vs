#version 330

in vec3 a_Pos;
uniform float u_Time;

out float v_Depth;

float c_PI = 3.141592;

void main()
{
	float value = a_Pos.x + 0.5f;
	float newX = a_Pos.x;
	float depth = value * sin(c_PI * (a_Pos.x + 0.5 ) * 2 - u_Time * 2);

	float newY = a_Pos.y + depth * 0.25;

	vec4 final = vec4(newX, newY, 0.0, 1.0);
	v_Depth = depth * 0.5 + 0.5;

	gl_Position = final; 
}