// 包含 GLEW 和 GLFW 的头文件
#include <GL/glew.h>       // GLEW：用于管理 OpenGL 扩展函数指针（必须在创建上下文后初始化）
#include <GLFW/glfw3.h>    // GLFW：用于创建窗口、处理 OpenGL 上下文和输入
#include <iostream>
#include <string>

// 自定义渲染器和封装类头文件
#include "Renderer.h"      // 渲染相关的辅助函数和宏（如 GLCall）
#include "VertexBuffer.h"  // 封装的顶点缓冲对象类
#include "IndexBuffer.h"   // 封装的索引缓冲对象类
#include "VertexArray.h"   // 封装的顶点数组对象类
#include "Shader.h"        // 封装的着色器类

int main(void)
{
    GLFWwindow* window;

    // 初始化 GLFW 库
    if (!glfwInit())
        return -1; // 初始化失败，程序退出

    // 创建一个窗口，大小为 640x480，标题为 "Hello World"
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate(); // 创建失败，清理资源
        return -1;
    }

    // 设置当前线程的 OpenGL 上下文为刚创建的窗口
    glfwMakeContextCurrent(window);

    // 启用垂直同步（防止撕裂，每帧刷新一次）
    glfwSwapInterval(1);

    // 初始化 GLEW（在上下文创建之后）
    if (glewInit() != GLEW_OK)
        std::cout << "Error initializing GLEW" << std::endl;

    // 输出当前的 OpenGL 版本
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    {
        // 定义四个顶点的位置坐标（x, y）构成一个矩形（以两个三角形绘制）
        float postions[] = {
            -0.5f, -0.5f, // 左下
             0.5f, -0.5f, // 右下
             0.5f,  0.5f, // 右上
            -0.5f,  0.5f  // 左上
        };

        // 定义绘制矩形的两个三角形的索引
        unsigned int indices[] = {
            0, 1, 2, // 第一个三角形
            2, 3, 0  // 第二个三角形
        };

        // 创建顶点数组对象（封装顶点属性）
        VertexArray va;

        // 创建顶点缓冲对象，绑定顶点数据（4个点 × 2维坐标 × float大小）
        VertexBuffer vb(postions, 4 * 2 * sizeof(float));

        // 创建顶点缓冲布局（每个顶点有 2 个 float 类型的分量）
        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout); // 将布局绑定到 VAO 上

        // 创建索引缓冲对象，绑定索引数据
        IndexBuffer ib(indices, 6);

        // 加载着色器程序，并进行绑定
        Shader shader("OpenGL/res/shaders/Basic.shader");
        shader.Bind();

        // 设置一个 uniform 变量的初始颜色
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

        // 解绑所有对象（防止之后误用）
        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        // 控制颜色变化的变量
        float r = 0.0f;
        float increment = 0.05f;

        // 主渲染循环
        while (!glfwWindowShouldClose(window))
        {
            // 清空颜色缓冲
            glClear(GL_COLOR_BUFFER_BIT);

            // 绑定着色器，并更新 uniform 颜色值
            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

            // 绑定 VAO 和索引缓冲准备绘制
            va.Bind();
            ib.Bind();

            // 调用封装的 GL 绘制宏，绘制两个三角形组成的矩形
            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            // 控制红色通道渐变
            if (r > 1.0f)
                increment = -0.05f;
            else if (r < 0.0f)
                increment = 0.05f;
            r += increment;

            // 交换前后缓冲，显示新帧
            glfwSwapBuffers(window);

            // 处理窗口事件（键盘鼠标等）
            glfwPollEvents();
        }
    }

    // 程序结束前清理资源
    glfwTerminate();
    return 0;
}
