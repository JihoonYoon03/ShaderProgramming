#pragma once

#include <string>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <array>

#include "Dependencies\glew.h"


class Renderer
{
public:
	Renderer(int windowSizeX, int windowSizeY);
	~Renderer();

	bool IsInitialized();
	void DrawSolidRect(float x, float y, float z, float size, float r, float g, float b, float a);
	void DrawTriangle();

	struct Particle
	{
		float centerX = 0;
		float centerY = 0;
		float size = 0.1;
		float mass = 1;
		float vx = 0;
		float vy = 0;
	};

private:
	std::default_random_engine dre{};
	std::uniform_real_distribution<float> urd{-3.f, 3.f};

	void Initialize(int windowSizeX, int windowSizeY);
	bool ReadFile(char* filename, std::string *target);
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShaders(char* filenameVS, char* filenameFS);
	void CreateVertexBufferObjects();
	void GetGLPosition(float x, float y, float *newX, float *newY);

	bool m_Initialized = false;
	
	unsigned int m_WindowSizeX = 0;
	unsigned int m_WindowSizeY = 0;

	GLuint m_VBORect = 0;
	GLuint m_SolidRectShader = 0;

	GLuint m_VBOTriangle = 0;
	GLuint m_TriangleShader = 0;

	GLuint m_VBOParticles = 0;

};

