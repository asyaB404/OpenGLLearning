// 包含 GLEW 和 GLFW 的头文件
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

struct ShaderProgramSource
{
	std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParesShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

	enum class ShaderType
	{
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
	};

    std::string line;
	std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (std::getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT;
            }
        }
        else
        {
			ss[(int)type] << line << '\n';
        }
    }

	return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
	int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        return 0;
    }
	return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
	unsigned int program = glCreateProgram();
	unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
	return program;
}

int main(void)
{
	GLFWwindow* window;

	/* 初始化 GLFW 库 */
	if (!glfwInit())
		return -1; // 初始化失败，直接返回

	/* 创建一个窗口和它的 OpenGL 上下文 */
	window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
	if (!window)
	{
		glfwTerminate(); // 创建窗口失败，清理 GLFW
		return -1;
	}

	/* 设置当前上下文为新创建的窗口 */
	glfwMakeContextCurrent(window);

	/* 初始化 GLEW（需要在 OpenGL 上下文之后初始化）*/
	if (glewInit() != GLEW_OK)
		std::cout << "Error initializing GLEW" << std::endl;

	// 输出当前使用的 OpenGL 版本
	std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

	// 三角形的顶点坐标（2D）
	float postions[6] = {
		-0.5f, -0.5f, // 左下角
		 0.0f,  0.5f, // 顶点
		 0.5f, -0.5f  // 右下角
	};

	/*
	好问题！👍
	我们来一步步深入浅出地解释：
	为什么要用 缓冲区（Buffer）？OpenGL 为什么不能直接画？这和“恶魔”（现代图形卡）的工作原理有关 😈

	🧠 一、为什么需要「缓冲」？
	在最早的 OpenGL（称为 Immediate Mode，即时模式）中，我们可以这样直接画图：

	cpp
	复制
	编辑
	glBegin(GL_TRIANGLES);
	glVertex2f(-0.5f, -0.5f);
	glVertex2f(0.0f, 0.5f);
	glVertex2f(0.5f, -0.5f);
	glEnd();
	这种方式：

	写一点 → 画一点。

	不保存 → 不缓存 → 不优化。

	👉 非常慢！几乎每一帧都得把数据重新交给 GPU。*/
	// 创建一个缓冲区对象（VBO：Vertex Buffer Object）
	unsigned int buffer; 
	glGenBuffers(1, &buffer); // 生成一个缓冲区 ID
	glBindBuffer(GL_ARRAY_BUFFER, buffer); // 绑定为当前数组缓冲
	glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(float), postions, GL_STATIC_DRAW); // 传入顶点数据

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    /*std::string vertexShader = R"(
        #version 330 core
        layout(location = 0) in vec4 position;
        void main()
        {
            gl_Position = position;
        }
    )";

    std::string fragmentShader = R"(
        #version 330 core
        out vec4 color;
        void main()
        {
            color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        }
    )";*/
    
	ShaderProgramSource source = ParesShader("OpenGL/res/shaders/Basic.shader");
    std::string vertexShader = source.VertexSource;
    std::string fragmentShader = source.FragmentSource;
	std::cout << vertexShader << std::endl;
	std::cout << fragmentShader << std::endl;
	unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader);


	/* 渲染循环：直到窗口被关闭 */
	while (!glfwWindowShouldClose(window))
	{
		// 清除颜色缓冲（用来清空屏幕）
		glClear(GL_COLOR_BUFFER_BIT);

		// 绘制三角形（从绑定的 VBO 读取 3 个顶点）
		glDrawArrays(GL_TRIANGLES, 0, 3);

		// --- 下面是旧版 Immediate 模式绘图方法（被现代 OpenGL 弃用） ---
		/*
		glBegin(GL_TRIANGLES);
		glVertex2f(-0.5f, -0.5f);
		glVertex2f(0.0f, 0.5f);
		glVertex2f(0.5f, -0.5f);
		glEnd();
		*/

		// 交换前后缓冲（将绘制结果显示到屏幕）
		glfwSwapBuffers(window);

		// 处理事件（如键盘、鼠标等输入）
		glfwPollEvents();
	}

	glDeleteProgram(shader);
	// 清理资源，关闭 GLFW
	glfwTerminate();
	return 0;
}
