#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"
#include <assert.h>

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
	m_FSShader = CompileShaders("./Shaders/FS.vs", "./Shaders/FS.fs");
	m_DummyShader = CompileShaders("./Shaders/Dummy.vs", "./Shaders/Dummy.fs");
	m_TextureShader = CompileShaders("./Shaders/Texture.vs", "./Shaders/Texture.fs");
	m_BlurH_Shader = CompileShaders("./Shaders/BlurH.vs", "./Shaders/BlurH.fs");
	m_BlurV_Shader = CompileShaders("./Shaders/BlurV.vs", "./Shaders/BlurV.fs");
	m_AccumShader = CompileShaders("./Shaders/Accum.vs", "./Shaders/Accum.fs");
	m_FullScreenColorShader = CompileShaders("./Shaders/FullScreenColor.vs", "./Shaders/FullScreenColor.fs");
	
	//Load Textures
	m_RgbTexture = CreatePngTexture ("./Textures/rgb.png", GL_NEAREST );	// slot 0
	m_NumsTexture = CreatePngTexture ("./Textures/numbers.png", GL_NEAREST );	// slot 1
	m_ParticleTexture = CreatePngTexture ("./Textures/particle.png", GL_NEAREST );	// slot 1
	m_ParticleSpriteTexture = CreatePngTexture ("./Textures/explosion.png", GL_NEAREST );	// slot 1
	m_AsdfTexture = CreatePngTexture ("./Textures/asdf.png", GL_NEAREST );	// slot 1

	for ( int i = 0; i < 10; ++i ) {
		std::string path = "./Textures/" + std::to_string ( i ) + ".png";
		m_NumTexture[i] = CreatePngTexture ((char*)path.c_str() , GL_NEAREST );	// slot 2~11
	}

	//Create VBOs
	CreateVertexBufferObjects();

	//Create Dummy Vertex
	GenDummyMesh ( 200 , 200 );

	//Gen Drop Info
	int index = 0;
	for (int i = 0; i < 1000; ++i) {
		float x = urd0_1(dre);
		float y = urd0_1(dre);
		float sTime = 3 * urd0_1(dre);
		float lTime = urd0_1(dre);

		m_DropPoints[index++] = x;
		m_DropPoints[index++] = y;
		m_DropPoints[index++] = sTime;
		m_DropPoints[index++] = lTime;
	}
	if (m_SolidRectShader > 0 && m_VBORect > 0)
	{
		m_Initialized = true;
	}

	GenFBOs ( );
}

GLuint Renderer::CreatePngTexture ( char* filePath , GLuint samplingMethod )
{
	//Load Png
	std::vector<unsigned char> image;

	unsigned width , height;
	unsigned error = lodepng::decode ( image , width , height , filePath );

	if ( error != 0 )
	{
		std::cout << "PNG image loading failed:" << filePath << std::endl;
		assert ( 0 );
	}

	GLuint temp;
	glGenTextures ( 1 , &temp );
	glBindTexture ( GL_TEXTURE_2D , temp );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA , width , height , 0 , GL_RGBA , GL_UNSIGNED_BYTE , &image[ 0 ] );

	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , samplingMethod );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , samplingMethod );

	return temp;
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

	float centerX = 0;
	float centerY = 0;
	float size = 0.025f;
	float mass = 1;	// kg
	float vx = 1;
	float vy = 1;
	// float RV0 = 0;
	// float RV1 = 0;

	float left = centerX - size / 2.0f;
	float right = centerX + size / 2.0f;
	float bottom = centerY - size / 2.0f;
	float top = centerY + size / 2.0f;

	float triangle[]
		=
	{
		left, bottom, 0.f,
		mass, vx, vy,
		right, bottom, 0.f,
		mass, vx, vy,
		right, top, 0.f,
		mass, vx, vy,	// Triangle 1

		left, bottom, 0.f,
		mass, vx, vy,
		right, top, 0.f,
		mass, vx, vy,
		left, top, 0.f,
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
	
	size = 0.1f;

	// 파티클 1개 = 정점 6개
	// 정점 1개 = x, y, z, mass, vx, vy, RV0, RV1, RV2, tx, ty, r, g, b -> 총 14개 float
	for (int i = 0; i < numParticles; ++i) {
		float vx_ = urd(dre);
		float vy_ = urd(dre);

		float RV0 = urd0_1(dre);
		float RV1 = urd0_1(dre);
		float RV2 = urd0_1(dre);

		float R = urd0_1 ( dre );
		float G = urd0_1 ( dre );
		float B = urd0_1 ( dre );

		float newParticle[] =
		{
			left, bottom, 0.f,
			mass, vx_, vy_, RV0, RV1, RV2, 0, 1, R, G, B,
			right, bottom, 0.f,
			mass, vx_, vy_, RV0, RV1, RV2, 1, 1, R, G, B,
			right, top, 0.f,
			mass, vx_, vy_, RV0, RV1, RV2, 1, 0, R, G, B,	// Triangle 1

			left, bottom, 0.f,
			mass, vx_, vy_, RV0, RV1, RV2, 0, 1, R, G, B,
			right, top, 0.f,
			mass, vx_, vy_, RV0, RV1, RV2, 1, 0, R, G, B,
			left, top, 0.f,
			mass, vx_, vy_, RV0, RV1, RV2, 0, 0, R, G, B		// Triangle 2
		};

		glBufferSubData(GL_ARRAY_BUFFER, sizeof(newParticle) * i, sizeof(newParticle), newParticle);

	}

	// x, y, z, tx, ty
	float rectFull[ ]
		=
	{
		-1.f, -1.f, 0.f, 0.f, 1.f,
		1.f, 1.f, 0.f, 1.f, 0.f,
		-1.f, 1.f, 0.f, 0.f, 0.f, //Triangle1

		-1.f, -1.f, 0.f, 0.f, 1.f,
		1.f, -1.f, 0.f, 1.f, 1.f,
		1.f, 1.f, 0.f, 1.f, 0.f,	//Triangle2
	};

	glGenBuffers ( 1 , &m_VBOFS );
	glBindBuffer ( GL_ARRAY_BUFFER , m_VBOFS );
	glBufferData ( GL_ARRAY_BUFFER , sizeof ( rectFull ) , rectFull , GL_STATIC_DRAW );

	float texRect[ ]
		=
	{
		-1.f, -1.f, 0.f,
		1.f, 1.f, 0.f, 
		-1.f, 1.f, 0.f, //Triangle1
		-1.f, -1.f, 0.f,
		1.f, 1.f, 0.f,
		1.f, -1.f, 0.f, //Triangle2
	};

	glGenBuffers ( 1 , &m_TextureVBO );
	glBindBuffer ( GL_ARRAY_BUFFER , m_TextureVBO );
	glBufferData ( GL_ARRAY_BUFFER , sizeof ( texRect ) , texRect , GL_STATIC_DRAW );

}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;
	GLint Lengths[1];
	Lengths[0] = strlen(pShaderText);
	glShaderSource(ShaderObj, 1, p, Lengths);

	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024];

		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s \n", pShaderText);
	}

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
	GLuint ShaderProgram = glCreateProgram();

	if (ShaderProgram == 0) {
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs, fs;

	if (!ReadFile(filenameVS, &vs)) {
		printf("Error compiling vertex shader\n");
		return -1;
	};

	if (!ReadFile(filenameFS, &fs)) {
		printf("Error compiling fragment shader\n");
		return -1;
	};

	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { 0 };

	glLinkProgram(ShaderProgram);

	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (Success == 0) {
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


void Renderer::GenDummyMesh ( int resolX , int resolY )
{
	float basePosX = -0.5f;
	float basePosY = -0.5f;
	float targetPosX = 0.5f;
	float targetPosY = 0.5f;

	int pointCountX = resolX + 1;
	int pointCountY = resolY + 1;
	float width = targetPosX - basePosX;
	float height = targetPosY - basePosY;

	float* point = new float[ pointCountX * pointCountY * 2 ];
	float* vertices = new float[ ( pointCountX - 1 ) * ( pointCountY - 1 ) * 2 * 3 * 3 ];
	m_VBODummyCount = ( pointCountX - 1 ) * ( pointCountY - 1 ) * 2 * 3; //Prepare points
	for ( int x = 0; x < pointCountX; x++ ) {
		for ( int y = 0; y < pointCountY; y++ ) {
			point[ ( y * pointCountX + x ) * 2 + 0 ] = basePosX + width * ( x / ( float ) ( pointCountX - 1 ) );
			point[ ( y * pointCountX + x ) * 2 + 1 ] = basePosY + height * ( y / ( float ) ( pointCountY - 1 ) );
		}
	}
	//Make triangles
	int vertIndex = 0;
	for ( int x = 0; x < pointCountX - 1; x++ ) {
		for ( int y = 0; y < pointCountY - 1; y++ ) {
			//Triangle part 1
			vertices[ vertIndex++ ] = point[ ( y * pointCountX + x ) * 2 + 0 ];
			vertices[ vertIndex++ ] = point[ ( y * pointCountX + x ) * 2 + 1 ];
			vertices[ vertIndex++ ] = 0.f;
			vertices[ vertIndex++ ] = point[ ( ( y + 1 ) * pointCountX + ( x + 1 ) ) * 2 + 0 ];
			vertices[ vertIndex++ ] = point[ ( ( y + 1 ) * pointCountX + ( x + 1 ) ) * 2 + 1 ];
			vertices[ vertIndex++ ] = 0.f;
			vertices[ vertIndex++ ] = point[ ( ( y + 1 ) * pointCountX + x ) * 2 + 0 ];
			vertices[ vertIndex++ ] = point[ ( ( y + 1 ) * pointCountX + x ) * 2 + 1 ];
			vertices[ vertIndex++ ] = 0.f;

			//Triangle part 2
			vertices[ vertIndex++ ] = point[ ( y * pointCountX + x ) * 2 + 0 ];
			vertices[ vertIndex++ ] = point[ ( y * pointCountX + x ) * 2 + 1 ];
			vertices[ vertIndex++ ] = 0.f;
			vertices[ vertIndex++ ] = point[ ( y * pointCountX + ( x + 1 ) ) * 2 + 0 ];
			vertices[ vertIndex++ ] = point[ ( y * pointCountX + ( x + 1 ) ) * 2 + 1 ];
			vertices[ vertIndex++ ] = 0.f;
			vertices[ vertIndex++ ] = point[ ( ( y + 1 ) * pointCountX + ( x + 1 ) ) * 2 + 0 ];
			vertices[ vertIndex++ ] = point[ ( ( y + 1 ) * pointCountX + ( x + 1 ) ) * 2 + 1 ];
			vertices[ vertIndex++ ] = 0.f;
		}
	}

	glGenBuffers ( 1 , &m_VBODummy );

	glBindBuffer ( GL_ARRAY_BUFFER , m_VBODummy );

	glBufferData ( GL_ARRAY_BUFFER , sizeof ( float ) * ( pointCountX - 1 ) * ( pointCountY - 1 ) * 2 * 3 * 3 , vertices , GL_STATIC_DRAW );
}

void Renderer::DrawTexture ( GLuint texID , float x , float y , float scale, bool flip )
{
	//Program select
	int shader = m_TextureShader;
	glUseProgram ( shader );

	int uTex = glGetUniformLocation ( shader , "u_Tex" );
	glUniform1i ( uTex , 0 );
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D , texID );
	int uTrans = glGetUniformLocation ( shader , "u_Trans" );
	glUniform4f ( uTrans , x , y , 1 , scale );

	int uFlip = glGetUniformLocation ( shader , "u_Flip" );
	glUniform1i ( uFlip , flip );


	int aPos = glGetAttribLocation ( shader , "a_Pos" );
	glEnableVertexAttribArray ( aPos );
	glBindBuffer ( GL_ARRAY_BUFFER , m_TextureVBO );
	glVertexAttribPointer ( aPos , 3 , GL_FLOAT , GL_FALSE , sizeof ( float ) * 3 , 0 );

	glDrawArrays ( GL_TRIANGLES , 0 , 6 );
}

void Renderer::GenFBOs ( )
{
	//Gen Texture
	glGenTextures ( 1 , &m_FBO_Texture );
	glBindTexture ( GL_TEXTURE_2D , m_FBO_Texture );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_GENERATE_MIPMAP , GL_TRUE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA8 , 340 , 340 , 0 , GL_RGBA , GL_UNSIGNED_BYTE , 0 );

	//Gen Render Buffer
	GLuint depthBuffer;
	glGenRenderbuffers ( 1 , &depthBuffer );
	glBindRenderbuffer ( GL_RENDERBUFFER , depthBuffer );
	glRenderbufferStorage ( GL_RENDERBUFFER , GL_DEPTH_COMPONENT , 340 , 340 );
	glBindRenderbuffer ( GL_RENDERBUFFER , 0 );

	//GenFBO
	glGenFramebuffers ( 1 , &m_FBO );
	//Binding before Attach
	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO );
	//Attach Color Buffer
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , m_FBO_Texture , 0 );
	//Attach Depth Buffer
	glFramebufferRenderbuffer ( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , depthBuffer );

	//Check status
	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) {
		assert ( 0 );
	}

	//Gen Texture
	glGenTextures ( 1 , &m_FBO_Texture1 );
	glBindTexture ( GL_TEXTURE_2D , m_FBO_Texture1 );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_GENERATE_MIPMAP , GL_TRUE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA8 , 340 , 340 , 0 , GL_RGBA , GL_UNSIGNED_BYTE , 0 );

	//Gen Render Buffer
	GLuint depthBuffer1;
	glGenRenderbuffers ( 1 , &depthBuffer1 );
	glBindRenderbuffer ( GL_RENDERBUFFER , depthBuffer1 );
	glRenderbufferStorage ( GL_RENDERBUFFER , GL_DEPTH_COMPONENT , 340 , 340 );
	glBindRenderbuffer ( GL_RENDERBUFFER , 0 );

	//GenFBO
	glGenFramebuffers ( 1 , &m_FBO1 );
	//Binding before Attach
	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO1 );
	//Attach Color Buffer
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , m_FBO_Texture1 , 0 );
	//Attach Depth Buffer
	glFramebufferRenderbuffer ( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , depthBuffer1 );

	//Check status
	status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) {
		assert ( 0 );
	}

	//Gen Texture
	glGenTextures ( 1 , &m_FBO_Texture2 );
	glBindTexture ( GL_TEXTURE_2D , m_FBO_Texture2 );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_GENERATE_MIPMAP , GL_TRUE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA8 , 340 , 340 , 0 , GL_RGBA , GL_UNSIGNED_BYTE , 0 );

	//Gen Render Buffer
	GLuint depthBuffer2;
	glGenRenderbuffers ( 1 , &depthBuffer2 );
	glBindRenderbuffer ( GL_RENDERBUFFER , depthBuffer2 );
	glRenderbufferStorage ( GL_RENDERBUFFER , GL_DEPTH_COMPONENT , 340 , 340 );
	glBindRenderbuffer ( GL_RENDERBUFFER , 0 );

	//GenFBO
	glGenFramebuffers ( 1 , &m_FBO2 );
	//Binding before Attach
	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO2 );
	//Attach Color Buffer
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , m_FBO_Texture2 , 0 );
	//Attach Depth Buffer
	glFramebufferRenderbuffer ( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , depthBuffer2 );

	//Check status
	status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) {
		assert ( 0 );
	}


	//Gen Texture
	glGenTextures ( 1 , &m_MRT_FBO_Texture0 );
	glBindTexture ( GL_TEXTURE_2D , m_MRT_FBO_Texture0 );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_GENERATE_MIPMAP , GL_TRUE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA8 , 340 , 340 , 0 , GL_RGBA , GL_UNSIGNED_BYTE , 0 );

	glGenTextures ( 1 , &m_MRT_FBO_Texture1 );
	glBindTexture ( GL_TEXTURE_2D , m_MRT_FBO_Texture1 );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_GENERATE_MIPMAP , GL_TRUE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA8 , 340 , 340 , 0 , GL_RGBA , GL_UNSIGNED_BYTE , 0 );

	glGenTextures ( 1 , &m_MRT_FBO_Texture2 );
	glBindTexture ( GL_TEXTURE_2D , m_MRT_FBO_Texture2 );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_GENERATE_MIPMAP , GL_TRUE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA8 , 340 , 340 , 0 , GL_RGBA , GL_UNSIGNED_BYTE , 0 );

	//Gen Render Buffer
	GLuint MRTdepthBuffer;
	glGenRenderbuffers ( 1 , &MRTdepthBuffer );
	glBindRenderbuffer ( GL_RENDERBUFFER , MRTdepthBuffer );
	glRenderbufferStorage ( GL_RENDERBUFFER , GL_DEPTH_COMPONENT , 340 , 340 );
	glBindRenderbuffer ( GL_RENDERBUFFER , 0 );

	//GenFBO
	glGenFramebuffers ( 1 , &m_MRT_FBO );
	//Binding before Attach
	glBindFramebuffer ( GL_FRAMEBUFFER , m_MRT_FBO );
	//Attach Color Buffer
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , m_MRT_FBO_Texture0 , 0 );
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D , m_MRT_FBO_Texture1 , 0 );
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT2 , GL_TEXTURE_2D , m_MRT_FBO_Texture2 , 0 );
	//Attach Depth Buffer
	glFramebufferRenderbuffer ( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT , GL_RENDERBUFFER , MRTdepthBuffer );

	status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	if ( status != GL_FRAMEBUFFER_COMPLETE ) {
		assert ( 0 );
	}

	glGenTextures ( 1 , &m_MRT_HDR_FBO_High_Texture );
	glBindTexture ( GL_TEXTURE_2D, m_MRT_HDR_FBO_High_Texture );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA16F , 640 , 640 , 0 , GL_RGBA , GL_FLOAT , NULL );

	glGenTextures ( 1 , &m_MRT_HDR_FBO_Low_Texture );
	glBindTexture ( GL_TEXTURE_2D, m_MRT_HDR_FBO_Low_Texture );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
	glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
	glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA16F , 640 , 640 , 0 , GL_RGBA , GL_FLOAT , NULL );

	glGenFramebuffers ( 1 , &m_MRT_HDR_FBO );
	glBindFramebuffer ( GL_FRAMEBUFFER , m_MRT_HDR_FBO );
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , m_MRT_HDR_FBO_Low_Texture , 0 );
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT1 , GL_TEXTURE_2D , m_MRT_HDR_FBO_High_Texture , 0 );

	glGenFramebuffers ( 2 , m_PingpongFBO );
	glGenTextures ( 2 , m_PingpongTexture );

	for ( int i = 0; i < 2; ++i ) {
		glBindTexture ( GL_TEXTURE_2D , m_PingpongTexture[ i ] );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_LINEAR );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP_TO_EDGE );
		glTexParameteri ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP_TO_EDGE );
		glTexImage2D ( GL_TEXTURE_2D , 0 , GL_RGBA16F , 640 , 640 , 0 , GL_RGBA , GL_FLOAT , NULL );

		glBindFramebuffer ( GL_FRAMEBUFFER , m_PingpongFBO[ i ] );
		glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 , GL_TEXTURE_2D , m_PingpongTexture[ i ] , 0 );

		status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
		if ( status != GL_FRAMEBUFFER_COMPLETE ) {
			assert ( 0 );
		}
	}

	glBindFramebuffer ( GL_FRAMEBUFFER , 0 );
}

void Renderer::DrawGaussianBlur ( GLuint texID , GLuint targetFBOID , GLuint shader )
{
	glBindFramebuffer ( GL_FRAMEBUFFER , targetFBOID );
	glUseProgram ( shader );

	GLuint posLoc = glGetAttribLocation ( shader , "a_Pos" );
	glEnableVertexAttribArray ( posLoc );
	GLuint texLoc = glGetAttribLocation ( shader , "a_Tex" );
	glEnableVertexAttribArray ( texLoc );
	glUniform1i ( glGetUniformLocation ( shader , "u_Texture" ) , 0 );
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D , texID );

	glBindBuffer ( GL_ARRAY_BUFFER , m_VBOFS );
	glVertexAttribPointer ( posLoc , 3 , GL_FLOAT , GL_FALSE , sizeof ( float ) * 5 , 0 );
	glVertexAttribPointer ( texLoc , 2 , GL_FLOAT , GL_FALSE , sizeof ( float ) * 5 , ( GLvoid* ) ( sizeof ( float ) * 3 ) );
	glDrawArrays ( GL_TRIANGLES , 0 , 6 );
	glBindFramebuffer ( GL_FRAMEBUFFER , 0 );

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

}

float g_time = 0;

//=======================
void Renderer::DrawTriangle()
//=======================
{
	g_time += 0.001;

	//Program select
	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time");
	glUniform1f(uTime, g_time);

	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Pos");
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

}

//=======================
void Renderer::DrawParticles()
//=======================
{
	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	QueryPerformanceCounter(&t_end);

	double elapsed = static_cast<double>(t_end.QuadPart - t_start.QuadPart) / t_frequency.QuadPart;

	QueryPerformanceCounter(&t_start);

	g_time += elapsed;

	//Program select
	glUseProgram(m_TriangleShader);

	int uTime = glGetUniformLocation(m_TriangleShader, "u_Time");
	glUniform1f(uTime, g_time);

	int uParticle = glGetUniformLocation ( m_TriangleShader , "u_ParticleTex" );
	glUniform1i ( uParticle , 0 );

	int uParticleSprite = glGetUniformLocation ( m_TriangleShader , "u_ParticleSpriteTex" );
	glUniform1i ( uParticleSprite , 1 );

	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D , m_ParticleTexture );
	glActiveTexture ( GL_TEXTURE1 );
	glBindTexture ( GL_TEXTURE_2D , m_ParticleSpriteTexture );

	int attribPosition = glGetAttribLocation(m_TriangleShader, "a_Pos");
	int attribMass = glGetAttribLocation(m_TriangleShader, "a_Mass");
	int attribVel = glGetAttribLocation(m_TriangleShader, "a_Vel");
	int attribRV0 = glGetAttribLocation(m_TriangleShader, "a_RV0");
	int attribRV1 = glGetAttribLocation(m_TriangleShader, "a_RV1");
	int attribRV2 = glGetAttribLocation(m_TriangleShader, "a_RV2");
	int attribTex = glGetAttribLocation(m_TriangleShader, "a_Tex");
	int attribRGB = glGetAttribLocation(m_TriangleShader, "a_RGB");
	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribMass);
	glEnableVertexAttribArray(attribVel);
	glEnableVertexAttribArray(attribRV0);
	glEnableVertexAttribArray(attribRV1);
	glEnableVertexAttribArray(attribRV2);
	glEnableVertexAttribArray(attribTex);
	glEnableVertexAttribArray(attribRGB);

	int stride = 14;

	// DX12 Root Signature처럼 Vertex shader 입력에 맞게 수정
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOParticles);
	glVertexAttribPointer(attribPosition,
		3, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * stride , 0);

	glVertexAttribPointer(attribMass,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * stride , (GLvoid*)(sizeof(float) * 3));

	glVertexAttribPointer(attribVel,
		2, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * stride , (GLvoid*)(sizeof(float) * 4));

	glVertexAttribPointer(attribRV0,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * stride , (GLvoid*)(sizeof(float) * 6));

	glVertexAttribPointer(attribRV1,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * stride , (GLvoid*)(sizeof(float) * 7));

	glVertexAttribPointer(attribRV2,
		1, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * stride , (GLvoid*)(sizeof(float) * 8));

	glVertexAttribPointer ( attribTex ,
		2 , GL_FLOAT,
		GL_FALSE,
		sizeof (float) * stride, (GLvoid*) (sizeof(float) * 9));

	glVertexAttribPointer ( attribRGB ,
		3 , GL_FLOAT ,
		GL_FALSE ,
		sizeof ( float ) * stride , ( GLvoid* ) ( sizeof ( float ) * 11 ) );

	glDrawArrays(GL_TRIANGLES, 0, 6 * numParticles);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribMass);
	glDisableVertexAttribArray(attribVel);
	glDisableVertexAttribArray(attribRV0);
	glDisableVertexAttribArray(attribRV1);
	glDisableVertexAttribArray(attribRV2);
	glDisableVertexAttribArray(attribTex);
	glDisableVertexAttribArray(attribRGB);


	glDisable ( GL_BLEND );
}

//======================= 프래그먼트 셰이더 테스트용 함수
int g_CurrNum = 0;
void Renderer::DrawFS()
//=======================
{
	QueryPerformanceCounter(&t_end);

	double elapsed = static_cast<double>(t_end.QuadPart - t_start.QuadPart) / t_frequency.QuadPart;

	QueryPerformanceCounter(&t_start);

	g_time += elapsed;

	GLuint shader = m_FSShader;
	glUseProgram(shader);

	int uTime = glGetUniformLocation(shader, "u_Time");
	glUniform1f(uTime, g_time);
	
	int uRGBTexture = glGetUniformLocation ( shader , "u_RGBTex" );
	glUniform1i ( uRGBTexture , 0 );

	int uCurrNumTexture = glGetUniformLocation ( shader , "u_CurrNumTex" );
	glUniform1i ( uCurrNumTexture , g_CurrNum + 2 );
	g_CurrNum += 1000;
	/*if ( ++g_CurrNum > 9 ) {
		g_CurrNum = 0;
	}*/
	//Sleep ( 500 );

	int uInputNum = glGetUniformLocation ( shader , "u_InputNum" );
	glUniform1i ( uInputNum , g_CurrNum );

	int uNumsTex = glGetUniformLocation ( shader , "u_NumsTex" );
	glUniform1i ( uNumsTex , 1 );

	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D , m_RgbTexture );

	glActiveTexture ( GL_TEXTURE1 );
	glBindTexture ( GL_TEXTURE_2D , m_NumsTexture );

	glActiveTexture ( GL_TEXTURE2 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[0] );
	glActiveTexture ( GL_TEXTURE3 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[1] );
	glActiveTexture ( GL_TEXTURE4 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[2] );
	glActiveTexture ( GL_TEXTURE5 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[3] );
	glActiveTexture ( GL_TEXTURE6 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[4] );
	glActiveTexture ( GL_TEXTURE7 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[5] );
	glActiveTexture ( GL_TEXTURE8 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[6] );
	glActiveTexture ( GL_TEXTURE9 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[7] );
	glActiveTexture ( GL_TEXTURE10 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[8] );
	glActiveTexture ( GL_TEXTURE11 );
	glBindTexture ( GL_TEXTURE_2D , m_NumTexture[9] );

	int uPoints = glGetUniformLocation(shader, "u_DropInfo");
	glUniform4fv(uPoints, 1000, m_DropPoints);

	int attribPosition = glGetAttribLocation(shader, "a_Pos");
	int attribTPos = glGetAttribLocation(shader, "a_TPos");
	glEnableVertexAttribArray(attribPosition);
	glEnableVertexAttribArray(attribTPos);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBOFS);
	glVertexAttribPointer(attribPosition,
		3, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 5, 0);
	
	glVertexAttribPointer(attribTPos,
		2, GL_FLOAT,
		GL_FALSE,
		sizeof(float) * 5, (GLvoid*)(sizeof(float) * 3));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribTPos);

}

void Renderer::DrawDummy ( )
{
	QueryPerformanceCounter ( &t_end );

	double elapsed = static_cast< double >( t_end.QuadPart - t_start.QuadPart ) / t_frequency.QuadPart;

	QueryPerformanceCounter ( &t_start );

	g_time += elapsed;

	//Program select
	int shader = m_DummyShader;
	glUseProgram ( shader );

	int attribPosition = glGetAttribLocation ( shader , "a_Pos" );
	int attribTime = glGetUniformLocation ( shader , "u_Time" );
	glUniform1f ( attribTime , g_time );

	int uAsdfTex = glGetUniformLocation( shader , "u_AsdfTex" );
	glUniform1i ( uAsdfTex , 0 );
	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D , m_AsdfTexture );

	int uPoints = glGetUniformLocation ( shader , "u_DropInfo" );
	glUniform4fv ( uPoints , 1000 , m_DropPoints );

	glEnableVertexAttribArray ( attribPosition );
	

	glBindBuffer ( GL_ARRAY_BUFFER , m_VBODummy );
	glVertexAttribPointer ( attribPosition , 3 , GL_FLOAT , GL_FALSE , sizeof ( float ) * 3 , 0 );

	glDrawArrays ( GL_TRIANGLES , 0 , m_VBODummyCount );

	glDisableVertexAttribArray ( attribPosition );

	DrawTexture ( m_RgbTexture , 0.5 , 0.5 , 0.1, true );
}

void Renderer::DrawDummy_FBO ( )
{
	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO );
	glViewport ( 0 , 0 , 512 , 512 );
	DrawDummy ( );
	glBindFramebuffer ( GL_FRAMEBUFFER , 0 );
	glViewport ( 0 , 0 , 640 , 640 );
	DrawTexture ( m_FBO_Texture , 0 , 0 , 0.5, false );
}

void Renderer::DrawAll_FBO ( )
{
	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO );
	//Bind가 되어있는 프레임 버퍼 클리어
	glClearColor ( 0.0f , 0.0f , 0.0f , 0.0f );
	glClearDepth ( 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport ( 0 , 0 , 320 , 320 );
	DrawFS ( );

	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO1 );
	glClearColor ( 0.0f , 0.0f , 0.0f , 0.0f );
	glClearDepth ( 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport ( 0 , 0 , 320 , 320 );
	DrawParticles ( );

	glBindFramebuffer ( GL_FRAMEBUFFER , m_FBO2 );
	glClearColor ( 0.0f , 0.0f , 0.0f , 0.0f );
	glClearDepth ( 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport ( 0 , 0 , 320 , 320 );
	DrawDummy ( );

	// 이 프레임 버퍼는 RenderScene에서 클리어 됨.
	glBindFramebuffer ( GL_FRAMEBUFFER , 0 );
	glViewport ( 0 , 0 , 640 , 640 );

	DrawTexture ( m_FBO_Texture , -0.5 , 0 , 0.3 , false );
	DrawTexture ( m_FBO_Texture1 , 0 , 0 , 0.3 , false );
	DrawTexture ( m_FBO_Texture2 , 0.5 , 0, 0.3 , false );
}

void Renderer::DrawMultipleRenderTarget ( )
{
	glBindFramebuffer ( GL_FRAMEBUFFER , m_MRT_FBO );
	GLenum DrawBuffers[ 3 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers ( 3 , DrawBuffers );
	glClearColor ( 0.0f , 0.0f , 0.0f , 0.0f );
	glClearDepth ( 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport ( 0 , 0 , 320 , 320 );

	DrawFS( );

	glBindFramebuffer ( GL_FRAMEBUFFER , 0 );
	glViewport ( 0 , 0 , 640 , 640 );

	GLenum ResetDrawBuffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers ( 1 , ResetDrawBuffers );

	DrawTexture ( m_MRT_FBO_Texture0 , -0.5 , 0 , 0.3 , false );
	DrawTexture ( m_MRT_FBO_Texture1 , 0.0 , 0 , 0.3 , false );
	DrawTexture ( m_MRT_FBO_Texture2 , 0.5 , 0 , 0.3 , false );
}

void Renderer::DrawFullScreenColor ( float r , float g , float b , float a )
{
	glEnable ( GL_BLEND );
	glBlendFunc ( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );

	int shader = m_FullScreenColorShader;
	glUseProgram ( shader );

	int uColor = glGetUniformLocation ( shader , "u_Color" );
	glUniform4f ( uColor , r , g , b , a );

	int aPos = glGetAttribLocation ( shader , "a_Pos" );
	glEnableVertexAttribArray ( aPos );
	glBindBuffer ( GL_ARRAY_BUFFER , m_TextureVBO );
	glVertexAttribPointer ( aPos , 3 , GL_FLOAT , GL_FALSE , sizeof ( float ) * 3 , 0 );

	glDrawArrays ( GL_TRIANGLES , 0 , 6 );

	glDisable ( GL_BLEND );
}

void Renderer::DrawTriangle_Bloom ( )
{
	glBindFramebuffer ( GL_FRAMEBUFFER , m_MRT_HDR_FBO );
	GLenum DrawBuffers[ 2 ] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers ( 2 , DrawBuffers );
	glClearColor ( 0.0f , 0.0f , 0.0f , 0.0f );
	glClearDepth ( 1.0f );
	glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glViewport ( 0 , 0 , 640 , 640 );

	DrawParticles ( );
	DrawGaussianBlur ( m_MRT_HDR_FBO_High_Texture , m_PingpongFBO[ 0 ] , m_BlurH_Shader );

	for ( int i = 0; i < 50; i++ )
	{
		DrawGaussianBlur ( m_PingpongTexture[ 0 ] , m_PingpongFBO[ 1 ] , m_BlurV_Shader );
		DrawGaussianBlur ( m_PingpongTexture[ 1 ] , m_PingpongFBO[ 0 ] , m_BlurH_Shader );
	}


	glBindFramebuffer ( GL_FRAMEBUFFER , 0 );
	glViewport ( 0 , 0 , 640 , 640 );

	GLenum ResetDrawBuffers[ 1 ] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers ( 1 , ResetDrawBuffers );

	DrawAccumResult ( m_MRT_HDR_FBO_Low_Texture , m_PingpongTexture[ 0 ] , false );

	DrawTexture ( m_MRT_HDR_FBO_Low_Texture , -0.5 , -0.8 , 0.2 , false );
	//DrawTexture ( m_MRT_HDR_FBO_High_Texture , 0.5 , 0.5 , 0.5 , false );

	DrawTexture ( m_PingpongTexture[ 0 ] , 0.5 , -0.8 , 0.2 , true );
	//DrawTexture ( m_PingpongTexture[ 1 ] , 0.5 , -0.5 , 0.5 , false );
}

void Renderer::DrawAccumResult ( GLuint texOri , GLuint texBlurred , bool bFlip )
{
	int shader = m_AccumShader;
	glUseProgram ( shader );

	int uFlip = glGetUniformLocation ( shader , "u_Flip" );
	glUniform1i ( uFlip , bFlip );
	int uTex = glGetUniformLocation ( shader , "u_Tex" );
	glUniform1i ( uTex , 0 );
	int uTexBlurred = glGetUniformLocation ( shader , "u_TexBlurred" );
	glUniform1i ( uTexBlurred , 1 );
	int uExposure = glGetUniformLocation ( shader , "u_Exposure" );
	glUniform1f ( uExposure , 2 );

	glActiveTexture ( GL_TEXTURE0 );
	glBindTexture ( GL_TEXTURE_2D , texOri );
	glActiveTexture ( GL_TEXTURE1 );
	glBindTexture ( GL_TEXTURE_2D , texBlurred );

	int aPos = glGetAttribLocation ( shader , "a_Pos" );
	glEnableVertexAttribArray ( aPos );

	glBindBuffer ( GL_ARRAY_BUFFER , m_TextureVBO );
	glVertexAttribPointer ( aPos , 3 , GL_FLOAT , GL_FALSE , sizeof(float) * 3 , 0 );

	glDrawArrays ( GL_TRIANGLES , 0 , 6 );
	glDisableVertexAttribArray ( aPos );
}

void Renderer::GetGLPosition(float x, float y, float* newX, float* newY)
{
	*newX = x * 2.f / m_WindowSizeX;
	*newY = y * 2.f / m_WindowSizeY;
}
