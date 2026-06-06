#version 330

in vec3 a_Pos;
uniform float u_Time;
uniform vec4 u_DropInfo[1000];

out float v_Grey;
out vec2 v_Tex;

float c_PI = 3.141592;

void Flag()
{
	float tX, tY;
	tX = a_Pos.x + 0.5;
	tY = 1.0 - (a_Pos.y + 0.5);
	v_Tex = vec2(tX, tY);

	float value = a_Pos.x + 0.5f;

	float newX = a_Pos.x;
	float newY = a_Pos.y * (1.0 - value*0.5) +
	0.25 * value * sin(c_PI * (a_Pos.x + 0.5 ) * 2 - u_Time * 2);

	vec4 final = vec4(newX, newY, 0.0, 1.0); 
	v_Grey = (sin(c_PI * (newX + 0.5 ) * 2 - u_Time) + 1.0) / 2.0;

	gl_Position = final; 
}

void Circles()
{
	float accum = 0;

	for(int i = 0; i < 1000; ++i)
	{
		vec2 center = u_DropInfo[i].xy - vec2(0.5, 0.5);
		vec2 pos = a_Pos.xy;
		float lTime = u_DropInfo[i].z;
		float sTime = u_DropInfo[i].w;
		float nTime = u_Time - sTime;

		if (nTime > 0)
		{
			float lVal = fract(nTime / lTime); // normalize
			float oneMinus = 1.0 - lVal;
			float t = lVal * lTime;
			float d = distance(center, pos);
			float range = t / 30.0;
			float fade = 15.0 * clamp(range - d, 0, 1.0);
			float sinValue = pow(abs(sin(d * 4 * c_PI * 8 - t * 2)), 3.0);

			accum += sinValue * fade * oneMinus;
		}
	}

	v_Grey = accum;

	//gl_Position = vec4(a_Pos, 1.0);
	
	gl_Position = vec4(a_Pos.x, a_Pos.y + accum * 0.02, a_Pos.z + accum, 1.0);
	// 실제로 z축이 accum에 따라 입체적으로 움직임. y축은 임시
}

void main()
{
	Circles();
}