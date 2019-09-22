// ImGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
// GLEW/GLAD
#include <glad/glad.h>
// GLFW
#include <GLFW/glfw3.h>
// OTHER
#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
// GLM
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "stb_image.h"
// homework
#include "donghua_game.h"
#include "donghua_gameobject.h"


// 函数声明
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void key_input(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void renderScene(Shader &shader, unsigned int depthMap, bool depthMode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void snakeMoveControl_input(GLFWwindow* window, int& movement);
void snakeMovementApply(int& movement, float distance);

const int WINDOW_WIDTH = 600;
const int WINDOW_HEIGHT = 600;


float lastX = WINDOW_WIDTH / 2.0f;
float lastY = WINDOW_HEIGHT / 2.0f;
bool firstTimeUsingMouse = true;

// meshes
unsigned int planeVAO;

GLfloat *cubeVertices = NULL;
unsigned int* cubeIndices = NULL;
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
unsigned int cubeEBO = 0;

Camera camera(WINDOW_WIDTH, WINDOW_HEIGHT, glm::vec3(0.0f, 8.0f, 9.0f));
Game game(20, 20);
GameObject gameObject(WINDOW_WIDTH, WINDOW_HEIGHT);

int main() {
	// 实例化GLFW窗口
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	// 抗锯齿
	glfwWindowHint(GLFW_SAMPLES, 4);
	// 创建一个窗口对象
	GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	// OpenGL渲染窗口的尺寸大小
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// 开始游戏循环之前注册函数至合适的回调(Esc退出)
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// 初始化GLEW/GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// 初始化ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsClassic();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 150");

	/*绘制图形*/
	//============================================
	// 开启深度测试
	glEnable(GL_DEPTH_TEST);
	// 开启抗锯齿
	glEnable(GL_MULTISAMPLE);
	// 着色器
	game.initShaders("normalShader", Phong_light_vertex, Phong_light_fragment);
	game.initShaders("simpleDepthShader", simpleDept_vertex, simpleDept_fragment);

	game.initTextures("floor", "./wood.png");
	game.initTextures("stone", "./brickwall.jpg");
	game.initTextures("food", "./food.png");
	game.initTextures("snakeBody", "./snake.png");
	game.initTextures("snakeHead", "./head.png");

	// 顶点数组
	GLfloat* vertices = NULL;
	GLfloat* planeVertices = NULL;

	int total_p_v = 0;
	std::vector<GLfloat> p_v;
	// 作图
	// plane
	p_v = gameObject.getPlaneVectices();
	total_p_v = p_v.size();
	planeVertices = new GLfloat[total_p_v];
	for (int i = 0; i < total_p_v; i++) {
		planeVertices[i] = p_v[i];
	}
	
	// 基础参数
	float cameraRadius = 10.0f, cameraAngle = 0.0f, objectAngle = 0.0f;
	float lastFrame = 0.0f, deltaTime = 0.0f;
	glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
	glm::vec3 lightPos(0.727f, 4.0f, 1.673f);

	// 载入平面
	GLuint planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * total_p_v, planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);

	// 创建深度缓冲
	const GLuint SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	GLuint depthMapFBO, depthMap;
	glGenFramebuffers(1, &depthMapFBO);
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	

	// 模式选择
	int modeChange = 1, orth_pers = 1;
	bool isAutoMove = false, isRestart = false;
	int movement = 1;
	float move_interval = 0, move_thres = 0.2f,
		  food_interval = 0, food_thres = 0.2f;
	// 默认颜色
	glm::vec4 clear_color = glm::vec4(0.172f, 0.173f, 0.172f, 1.00f);

	// 游戏循环
	while (!glfwWindowShouldClose(window)) {
		// 检查事件
		glfwPollEvents();
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		key_input(window, deltaTime);
		snakeMoveControl_input(window, movement);
		move_interval += deltaTime;
		food_interval += deltaTime;
		if (!game.isFinish && move_interval > (0.4 - move_thres)) {
			snakeMovementApply(movement, 1.0f);
			move_interval = 0;
		}
		if (!game.isFinish && food_interval > (1.0f / food_thres)) {
			game.randomGenFood();
			food_interval = 0;
		}

		// ImGUI控制部分
		// ImGUI生成
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 设置GUI窗口内容
		ImGui::Begin("Options");
		ImGui::SliderFloat("Snake speed", &move_thres, 0.1, 0.3);
		ImGui::SliderFloat("Food/sec", &food_thres, 0.2, 1);
		ImGui::End();

		if (game.isFinish) {
			ImGui::Begin("Information");
			isRestart = ImGui::Button("Restart", ImVec2(60.0f, 30.0f));
			ImGui::End();
		}
		// 渲染指令
		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		// 清除窗口的移动轨迹
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		// 清空深度测试缓存
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 重来
		if (isRestart) {
			game.reset();
			isRestart = false;
			movement = 1;
			move_interval = 0;
			food_interval = 0;
		}

		/*渲染阴影贴图*/
		// 计算光空间转换矩阵
		glm::mat4 lightProjection = glm::mat4(1.0f), 
				lightView = glm::mat4(1.0f),
				lightSpaceMatrix = glm::mat4(1.0f);
		GLfloat near_plane = 1.0f, far_plane = 10.0f;
		
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		// 渲染阴影贴图
		game.getShaderByName("simpleDepthShader")->use();
		game.getShaderByName("simpleDepthShader")->bindMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
			glClear(GL_DEPTH_BUFFER_BIT);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, game.getTextureByName("floor")->getTexture());
			// 消除悬浮
			glCullFace(GL_FRONT);
			renderScene(*(game.getShaderByName("simpleDepthShader")), depthMap, true);
			// 设回原先的culling face
			glCullFace(GL_BACK); 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// 渲染正常场景
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		game.getShaderByName("normalShader")->use();
		glm::vec3 openShadow(modeChange);
		glm::mat4 projection = glm::perspective(glm::radians(30.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = glm::lookAt(glm::vec3(-0.25f, 20.0f, 6.0f), glm::vec3(-0.25f, 0.0f, -5.0f), glm::vec3(0.0, 1.0, 0.0));//camera.getView();//
		glm::vec3 camPos = camera.getPosition();
		// 设置参数
		game.getShaderByName("normalShader")->bindVec3("openShadow", openShadow);
		game.getShaderByName("normalShader")->bindInt("diffuseTexture", 0);
		game.getShaderByName("normalShader")->bindInt("shadowMap", 1);
		game.getShaderByName("normalShader")->bindMat4("projection", projection);
		game.getShaderByName("normalShader")->bindMat4("view", view);
		game.getShaderByName("normalShader")->bindVec3("viewPos", camPos);
		game.getShaderByName("normalShader")->bindVec3("lightPos", lightPos);
		game.getShaderByName("normalShader")->bindMat4("lightSpaceMatrix", lightSpaceMatrix);

		renderScene(*(game.getShaderByName("normalShader")), depthMap, false);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwMakeContextCurrent(window);

		// 交换缓冲
		glfwSwapBuffers(window);
	}
	
	delete[] planeVertices;
	delete[] cubeVertices;

	// 释放VBO，VAO，EBO
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	//============================================

	// 释放GLFW分配的内存
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	// 当用户按下ESC键,我们设置window窗口的WindowShouldClose属性为true
	// 关闭应用程序
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
}

void key_input(GLFWwindow* window, float deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
		camera.ProcessKeyboard(UP, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
		camera.ProcessKeyboard(DOWN, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
}

void snakeMoveControl_input(GLFWwindow* window, int& movement) {
	int temp_move = movement;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		//game.processSnakeMove(UP);
		movement = 0;
	}
	else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		//game.processSnakeMove(LEFT);
		movement = 1;
	}
	else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		//game.processSnakeMove(DOWN);
		movement = 3;
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		//game.processSnakeMove(RIGHT);
		movement = 2;
	}
	if (movement + temp_move == 3) {
		movement = temp_move;
	}
}

void snakeMovementApply(int& movement, float distance) {
	if (movement == 0) {
		game.processSnakeMove(UP, distance);
	}
	else if (movement == 1) {
		game.processSnakeMove(LEFT, distance);
	}
	else if (movement == 3) {
		game.processSnakeMove(DOWN, distance);
	}
	else if (movement == 2) {
		game.processSnakeMove(RIGHT, distance);
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstTimeUsingMouse) {
		lastX = xpos;
		lastY = ypos;
		firstTimeUsingMouse = false;
	}

	float xoffset = xpos - lastX,
		yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMove(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

void renderScene(Shader &shader, unsigned int depthMap, bool depthMode){
	game.setupObjectTexture(shader, depthMap, "floor");
	// 渲染平面
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-0.25f, 0.2f, -5.2f)) * glm::scale(model, glm::vec3(0.21f, 1.0f, 0.2f));
	shader.bindMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	// 渲染立方体
	std::vector<glm::mat4> stoneList = game.getRenderStoneMatrix(),
						   foodList = game.getRenderFoodMatrix(),
						   snake = game.getRenderSnakeMatrix();
	// stone
	if(!depthMode)
		game.setupObjectTexture(shader, depthMap, "stone");
	for(int i = 0; i < stoneList.size(); i++){
		shader.bindMat4("model", stoneList[i]);
		gameObject.renderCube(cubeVAO, cubeVBO, cubeVertices);
	}
	
	// food
	if(!depthMode)
		game.setupObjectTexture(shader, depthMap, "food");
	for (int i = 0; i < foodList.size(); i++) {
		model = foodList[i];
		shader.bindMat4("model", model);
		gameObject.renderCube(cubeVAO, cubeVBO, cubeVertices);
	}
	// snakeHead
	if(!depthMode)
		game.setupObjectTexture(shader, depthMap, "snakeHead");
	shader.bindMat4("model", snake[0]);
	gameObject.renderCube(cubeVAO, cubeVBO, cubeVertices);

	// snakeBody
	if(!depthMode)
		game.setupObjectTexture(shader, depthMap, "snakeBody");
	for (int i = 1; i < snake.size(); i++) {
		shader.bindMat4("model", snake[i]);
		gameObject.renderCube(cubeVAO, cubeVBO, cubeVertices);
	}
}