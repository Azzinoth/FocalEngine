//#include "Renderer\FEEntity.h"
#include "Renderer\FERenderer.h"

#define WIN_W 1280
#define WIN_H 720

float minX = -0.5f;
float minY = -0.5f;
float minZ = -0.5f;

float maxX = 0.5f;
float maxY = 0.5f;
float maxZ = 0.5f;

std::vector<float> VERTICES = {
	maxX, maxY, minZ, // 3
	maxX, minY, minZ, // 2
	minX, minY, minZ, // 1
	minX, minY, minZ, // 1
	minX, maxY, minZ, // 0
	maxX, maxY, minZ, // 3

	minX, maxY, minZ, // 0
	minX, minY, minZ, // 1
	minX, minY, maxZ, // 4
	minX, minY, maxZ, // 4
	minX, maxY, maxZ, // 5
	minX, maxY, minZ, // 0

	maxX, maxY, maxZ, // 7
	maxX, minY, maxZ, // 6
	maxX, minY, minZ, // 2
	maxX, minY, minZ, // 2
	maxX, maxY, minZ, // 3
	maxX, maxY, maxZ, // 7

	minX, maxY, maxZ, // 5
	minX, minY, maxZ, // 4
	maxX, maxY, maxZ, // 7
	maxX, maxY, maxZ, // 7
	minX, minY, maxZ, // 4
	maxX, minY, maxZ, // 6

	maxX, maxY, minZ, // 3
	minX, maxY, minZ, // 0
	minX, maxY, maxZ, // 5
	minX, maxY, maxZ, // 5
	maxX, maxY, maxZ, // 7
	maxX, maxY, minZ, // 3

	minX, minY, minZ, // 1
	maxX, minY, minZ, // 2
	maxX, minY, maxZ, // 6
	maxX, minY, maxZ, // 6
	minX, minY, maxZ, // 4
	minX, minY, minZ  // 1
};

//std::vector<float> VERTICES = {
//	-0.5f, -0.5f, 0.0f,
//	0.5f, -0.5f, 0.0f,
//	0.0f,  0.5f, 0.0f
//};

//static const char* vertex_shader_text =
//"#version 110\n"
//"uniform mat4 MVP;\n"
//"attribute vec3 vCol;\n"
//"attribute vec2 vPos;\n"
//"varying vec3 color;\n"
//"void main()\n"
//"{\n"
//"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
//"    color = vCol;\n"
//"}\n";
//static const char* fragment_shader_text =
//"#version 110\n"
//"varying vec3 color;\n"
//"void main()\n"
//"{\n"
//"    gl_FragColor = vec4(color, 1.0);\n"
//"}\n";
//static void error_callback(int error, const char* description)
//{
//	fprintf(stderr, "Error: %s\n", description);
//}

static const char* const sTestVS = R"(
#version 400 core

in vec3 vPos;
uniform mat4 ProjectionMatrix;

void main(void)
{
	gl_Position = ProjectionMatrix * vec4(vPos, 1.0);
}
)";

static const char* const sTestFS = R"(
#version 400 core

//out vec4 out_Color;

void main(void)
{
	gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
)";

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	GLFWwindow* window;
	
	//GLuint vertex_buffer, vertex_shader, fragment_shader, program;
	//GLint mvp_location, vpos_location, vcol_location;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(WIN_W, WIN_H, "Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Make the window's context current */
	glfwMakeContextCurrent(window);
	// get rid of console window
	//FreeConsole();
	glewInit();

	//glGenBuffers(1, &vertex_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	//glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
	//glCompileShader(vertex_shader);
	//fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	//glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
	//glCompileShader(fragment_shader);
	//program = glCreateProgram();
	//glAttachShader(program, vertex_shader);
	//glAttachShader(program, fragment_shader);
	//glLinkProgram(program);

	//mvp_location = glGetUniformLocation(program, "MVP");
	//vpos_location = glGetAttribLocation(program, "vPos");
	//vcol_location = glGetAttribLocation(program, "vCol");

	//glEnableVertexAttribArray(vpos_location);
	//glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
	//	sizeof(vertices[0]), (void*)0);
	//glEnableVertexAttribArray(vcol_location);
	//glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
	//	sizeof(vertices[0]), (void*)(sizeof(float) * 2));

	FocalEngine::FEShader* testShader = new FocalEngine::FEShader(sTestVS, sTestFS, std::vector<std::string> {"vPos"});
	FocalEngine::FEMaterial* testMaterial = new FocalEngine::FEMaterial();
	testMaterial->shaders.push_back(testShader);

	//createVAO();
	GLuint vaoID;
	glGenVertexArrays(1, &vaoID);
	glBindVertexArray(vaoID);

	GLenum error = glGetError();
	//storeDataInAttributeList(0, dimensions, positions);
	GLuint vboID;
	glGenBuffers(1, &vboID);
	glBindBuffer(GL_ARRAY_BUFFER, vboID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * VERTICES.size(), VERTICES.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	error = glGetError();
	//unbindVAO();
	glBindVertexArray(0);


	FocalEngine::FEMesh* testCube = new FocalEngine::FEMesh(vaoID, VERTICES.size() / 3);
	FocalEngine::FEEntity* testEntity = new FocalEngine::FEEntity(testCube, testMaterial);

	FocalEngine::FEEntity* testEntity2 = new FocalEngine::FEEntity(testCube, testMaterial);
	testEntity2->setPosition(glm::vec3(1.5f, 0.0f, 0.0f));

	//FocalEngine::FERenderer* Renderer = FocalEngine::FERenderer::getInstance();

	FocalEngine::FERenderer renderer = FocalEngine::FERenderer::getInstance();
	renderer.addToScene(testEntity);
	renderer.addToScene(testEntity2);

	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//glEnable(GL_DEPTH_TEST);

		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderer.render();
		//testEntity->render();

		//float ratio;
		//int width, height;
		//mat4x4 m, p, mvp;
		//glfwGetFramebufferSize(window, &width, &height);
		//ratio = width / (float)height;
		//glViewport(0, 0, width, height);
		//glClear(GL_COLOR_BUFFER_BIT);
		//mat4x4_identity(m);
		//mat4x4_rotate_Z(m, m, (float)glfwGetTime());
		//mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
		//mat4x4_mul(mvp, p, m);
		//glUseProgram(program);
		//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
		//glDrawArrays(GL_TRIANGLES, 0, 3);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}