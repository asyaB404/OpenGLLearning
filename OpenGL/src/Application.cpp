// 包含 GLEW 和 GLFW 的头文件
#include <GL/glew.h>       // GLEW 用于加载 OpenGL 扩展函数
#include <GLFW/glfw3.h>    // GLFW 用于创建窗口和处理输入
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x,__FILE__,__LINE__))

static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
        return false;
    }
    return true;
}

// ShaderProgramSource 结构体存储从文件读取的着色器代码
struct ShaderProgramSource
{
    std::string VertexSource;   // 顶点着色器代码
    std::string FragmentSource; // 片段着色器代码
};

// 解析着色器文件
static ShaderProgramSource ParesShader(const std::string& filepath)
{
    std::ifstream stream(filepath); // 打开文件

    // 定义着色器类型
    enum class ShaderType
    {
        NONE = -1,   // 无效类型
        VERTEX = 0,  // 顶点着色器
        FRAGMENT = 1 // 片段着色器
    };

    std::string line;            // 存储每行的内容
    std::stringstream ss[2];     // 存储顶点着色器和片段着色器的代码
    ShaderType type = ShaderType::NONE; // 初始时类型为 NONE

    // 逐行读取文件内容
    while (std::getline(stream, line))
    {
        // 判断是否是着色器标记
        if (line.find("#shader") != std::string::npos)
        {
            // 根据 "#shader" 标签判断着色器类型
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::VERTEX; // 顶点着色器
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::FRAGMENT; // 片段着色器
            }
        }
        else
        {
            // 将每行代码写入对应的着色器
            ss[(int)type] << line << '\n';
        }
    }

    // 返回包含顶点和片段着色器代码的结构体
    return { ss[0].str(), ss[1].str() };
}

// 编译单个着色器
static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);  // 创建着色器
    const char* src = source.c_str();        // 转换为 c 字符串
    glShaderSource(id, 1, &src, nullptr);    // 设置着色器源代码
    glCompileShader(id);                     // 编译着色器

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); // 获取编译状态
    if (result == GL_FALSE) // 如果编译失败
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);  // 获取错误信息长度
        char* message = (char*)alloca(length * sizeof(char)); // 分配内存
        glGetShaderInfoLog(id, length, &length, message);  // 获取错误信息
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << message << std::endl;  // 打印错误信息
        glDeleteShader(id);  // 删除着色器
        return 0;  // 返回 0 表示编译失败
    }
    return id; // 返回编译成功的着色器 ID
}

// 创建着色器程序
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram(); // 创建着色器程序
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);  // 编译顶点着色器
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader); // 编译片段着色器
    glAttachShader(program, vs);  // 将顶点着色器附加到程序
    glAttachShader(program, fs);  // 将片段着色器附加到程序
    glLinkProgram(program);       // 链接着色器程序
    glValidateProgram(program);   // 验证程序是否有效

    glDeleteShader(vs);  // 删除顶点着色器
    glDeleteShader(fs);  // 删除片段着色器
    return program;      // 返回着色器程序 ID
}

int main(void)
{
    GLFWwindow* window;

    // 初始化 GLFW 库
    if (!glfwInit())
        return -1; // 初始化失败，直接返回

    // 创建一个窗口和它的 OpenGL 上下文
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate(); // 创建窗口失败，清理 GLFW
        return -1;
    }

    // 设置当前上下文为新创建的窗口
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1);

    // 初始化 GLEW（需要在 OpenGL 上下文之后初始化）
    if (glewInit() != GLEW_OK)
        std::cout << "Error initializing GLEW" << std::endl;

    // 输出当前使用的 OpenGL 版本
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // 三角形的顶点坐标（2D）
    float postions[] = {
        -0.5f, -0.5f, // Left bottom
         0.5f, -0.5f, // Right bottom
         0.5f,  0.5f, // Right top
        -0.5f,  0.5f  // Left top
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    /*
    我们来一步步深入浅出地解释：
    为什么要用 缓冲区（Buffer）？OpenGL 为什么不能直接画？这和“恶魔”（现代图形卡）的工作原理有关 😈

    🧠 一、为什么需要「缓冲」？
    在最早的 OpenGL（称为 Immediate Mode，即时模式）中，我们可以这样直接画图：

  
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
	GLCall(glGenBuffers(1, &buffer);) // 生成一个缓冲区 ID
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer)); // 绑定为当前数组缓冲
    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), postions, GL_STATIC_DRAW)); // 传入顶点数据

    // 启用顶点属性数组（此处为位置数据）
    GLCall(glEnableVertexAttribArray(0));
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6  * sizeof(unsigned int), indices, GL_STATIC_DRAW));

    // 从着色器文件中加载顶点和片段着色器代码
    ShaderProgramSource source = ParesShader("OpenGL/res/shaders/Basic.shader");

    // 创建着色器程序并使用它
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.5f, 0.3f, 0.8f, 1.0f));

    float r = 0.0f;
    float increment = 0.05f;

    // 渲染循环：直到窗口被关闭
    while (!glfwWindowShouldClose(window))
    {
        // 清除颜色缓冲（用来清空屏幕）
        glClear(GL_COLOR_BUFFER_BIT);

        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment;

        // 交换前后缓冲（将绘制结果显示到屏幕）
        glfwSwapBuffers(window);

        // 处理事件（如键盘、鼠标等输入）
        glfwPollEvents();
    }

    // 删除着色器程序
    glDeleteProgram(shader);

    // 清理资源，关闭 GLFW
    glfwTerminate();
    return 0;
}
