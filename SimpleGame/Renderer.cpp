#include "stdafx.h"
#include "Renderer.h"

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	Initialize(windowSizeX, windowSizeY);

	QueryPerformanceFrequency(&t_frequency);

	QueryPerformanceCounter(&t_start);

	QueryPerformanceCounter(&t_end);
}


Renderer::~Renderer()
{
}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{


	//Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	//Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");
	m_TriangleShader = CompileShaders("./Shaders/Triangle.vs", "./Shaders/Triangle.fs");

	//Create VBOs
	CreateVertexBufferObjects();

	if (m_SolidRectShader > 0 && m_VBORect > 0)
	{
		m_Initialized = true;
	}
}

bool Renderer::IsInitialized()
{
	return m_Initialized;
}


void Renderer::CreateVertexBufferObjects()
{
	float rect[]
		=
	{
		-1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f, -1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, 1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, //Triangle1
		-1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f,  1.f / m_WindowSizeX, 1.f / m_WindowSizeY, 0.f, 1.f / m_WindowSizeX, -1.f / m_WindowSizeY, 0.f, //Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);

	// 정점 1개 = x, y, z, mass, vx, vy, RV0, RV1 -> 총 8개 float
	float centerX = 0;
	float centerY = 0;
	float size = 0.1;
	float mass = 1;	// kg
	float vx = 1;
	float vy = 1;
	// float RV0 = 0;
	// float RV1 = 0;

	float triangle[]
		=
	{
		centerX - size / 2, centerY - size / 2, 0.f,
		mass, vx, vy,
		centerX + size / 2, centerY - size / 2, 0.f,
		mass, vx, vy,
		centerX + size / 2, centerY + size / 2, 0.f,
		mass, vx, vy,	// Triangle 1

		centerX - size / 2, centerY - size / 2, 0.f,
		mass, vx, vy,
		centerX + size / 2, centerY + size / 2, 0.f,
		mass, vx, vy,
		centerX - size / 2, centerY + size / 2, 0.f,
		mass, vx, vy	// Triangle 2
	};

	// VBO 아이디 지정 및 바인딩
	glGenBuffers(1, &m_VBOTriangle);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOTriangle);

	// glBufferData에서 m_VBOTriangle에 실제 메모리 주소 저장한다고 보면 됨
	// 동기식으로 작동. 데이터 올라갈 때 까지 기다림
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);

	// VBO 아이디 지정 및 바인딩
	glGenBuffers(1, &m_VBOParticles);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOParticles);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangle) * numParticles, nullptr, GL_STATIC_DRAW);
	
	size = 0.05f;

	for (int i = 0; i < numParticles; ++i) {
		float vx_ = urd0_1(dre);
		float vy_ = urd0_1(dre);

		float RV0 = urd0_1(dre);
		float RV1 = urd0_1(dre);

		float newParticle[] =
		{
			centerX - size / 2, centerY - size / 2, 0.f,
			mass, vx_, vy_, RV0, RV1,
			centerX + size / 2, centerY - size / 2, 0.f,
			mass, vx_, vy_, RV0, RV1,
			centerX + size / 2, centerY + size / 2, 0.f,
			mass, vx_, vy_, RV0, RV1,	// Triangle 1

			centerX - size / 2, centerY - size / 2, 0.f,
			mass, vx_, vy_, RV0, RV1,
			centerX + size / 2, centerY + size / 2, 0.f,
			mass, vx_, vy_, RV0, RV1,
			centerX - size / 2, centerY + size / 2, 0.f,
			mass, vx_, vy_, RV0, RV1	// Triangle 2
		};

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(newParticle) * i, sizeof(newParticle), newParticle);
	}
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	//���̴� ������Ʈ ����
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	//���̴� �ڵ带 ���̴� ������Ʈ�� �Ҵ�
	glShaderSource(ShaderObj, 1, p, Lengths);

	//�Ҵ�� ���̴� �ڵ带 ������
	glCompileShader(ShaderObj);

	GLint success;
	// ShaderObj �� ���������� ������ �Ǿ����� Ȯ��
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		//OpenGL �� shader log �����͸� ������
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

	// ShaderProgram �� attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

bool Renderer::ReadFile(char* filename, std::string* target)
{
	std::ifstream file(filename);
	if (file.fail())
	{
		std::cout << filename << " file loading failed.. \n";
		file.close();
		return false;
	}
	std::string line;
	while (getline(file, line)) {
		target->append(line.c_str());
		target->append("\n");
	}
	return true;
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	GLuint ShaderProgram = glCreateProgram(); //�� ���̴� ���α׷� ����

	if (ShaderProgram == 0) { //���̴� ���α׷��� ����������� Ȯ��
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	//shader.vs �� vs ������ �ε���
	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	//shader.fs �� fs ������ �ε���
	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	// ShaderProgram �� vs.c_str() ���ؽ� ���̴��� �������� ����� attach��
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram �� fs.c_str() �����׸�Ʈ ���̴��� �������� ����� attach��
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	//Attach �Ϸ�� shaderProgram �� ��ŷ��
	glLinkProgram(ShaderProgram);

	//��ũ�� �����ߴ��� Ȯ��
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
		// shader program �α׸� �޾ƿ�
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error linking shader program\n" << ErrorLog;
		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << " Error validating shader program\n" << ErrorLog;
		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << " Shader compiling is done.";

	return ShaderProgram;
}

void Renderer::DrawSolidRect(float x, float y, float z, float size, float r, float g, float b, float a)
{
	float newX, newY;

	GetGLPosition(x, y, &newX, &newY);

	//Program select
	glUseProgram(m_SolidRectShader);

	glUniform4f(glGetUniformLocation(m_SolidRectShader, "u_Trans"), newX, newY, 0, size);
	glUniform4f(glGetUniformLocation(m_SolidRectShader, "u_Color"), r, g, b, a);

	int attribPosition = glGetAttribLocation(m_SolidRectShader, "a_Position");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float g_time = 0;

void Renderer::DrawTriangle()
{
	g_time += 0.001;

	//Program select
	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time");
	glUniform1f(uTime, g_time);

	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Position");
	int attribMass = glGetAttribLocation(m_TriangleShader, "a_Mass");
	int attribVel = glGetAttribLocation(m_TriangleShader, "a_Vel");
	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribMass);
	glEnableVertexAttribArray(attribVel);

	// DX12 Root Signature처럼 Vertex shader 입력에 맞게 수정
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOTriangle);
	glVertexAttribPointer(attribPosition,
		3, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 6, 0);

	glVertexAttribPointer(attribMass,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 6, (GLvoid*)(sizeof(float) * 3));

	glVertexAttribPointer(attribVel,
		2, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 6, (GLvoid*)(sizeof(float) * 4));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribMass);
	glDisableVertexAttribArray(attribVel);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawParticles()
{
	QueryPerformanceCounter(&t_end);

	double elapsed = static_cast<double>(t_end.QuadPart - t_start.QuadPart) / t_frequency.QuadPart;

	QueryPerformanceCounter(&t_start);

	g_time += elapsed;

	//Program select
	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time");
	glUniform1f(uTime, g_time);

	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Position");
	int attribMass = glGetAttribLocation(m_TriangleShader, "a_Mass");
	int attribVel = glGetAttribLocation(m_TriangleShader, "a_Vel");
	int attribRV0 = glGetAttribLocation(m_TriangleShader, "a_RV0");
	int attribRV1 = glGetAttribLocation(m_TriangleShader, "a_RV1");
	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribMass);
	glEnableVertexAttribArray(attribVel);
	glEnableVertexAttribArray(attribRV0);
	glEnableVertexAttribArray(attribRV1);

	// DX12 Root Signature처럼 Vertex shader 입력에 맞게 수정
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOParticles);
	glVertexAttribPointer(attribPosition,
		3, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8, 0);

	glVertexAttribPointer(attribMass,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8, (GLvoid*)(sizeof(float) * 3));

	glVertexAttribPointer(attribVel,
		2, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8, (GLvoid*)(sizeof(float) * 4));

	glVertexAttribPointer(attribRV0,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8, (GLvoid*)(sizeof(float) * 6));

	glVertexAttribPointer(attribRV1,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 8, (GLvoid*)(sizeof(float) * 7));

	glDrawArrays(GL_TRIANGLES, 0, 6 * numParticles);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribMass);
	glDisableVertexAttribArray(attribVel);
	glDisableVertexAttribArray(attribRV0);
	glDisableVertexAttribArray(attribRV1);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::GetGLPosition(float x, float y, float* newX, float* newY)
{
	*newX = x * 2.f / m_WindowSizeX;
	*newY = y * 2.f / m_WindowSizeY;
}