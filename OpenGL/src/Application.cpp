// 包含 GLEW 和 GLFW 的头文件
#include <GL/glew.h>       // GLEW 用于加载 OpenGL 扩展函数
#include <GLFW/glfw3.h>    // GLFW 用于创建窗口和处理输入
#include <iostream>
#include <string>

#include "Renderer.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"

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
    {
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

        unsigned int vao;
        GLCall(glGenVertexArrays(1, &vao));
        GLCall(glBindVertexArray(vao));
        VertexArray va;
        VertexBuffer vb(postions, 4 * 2 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        Shader shader("OpenGL/res/shaders/Basic.shader");
        shader.Bind();
        shader.SetUniform4f("u_Color", 0.8f, 0.3f, 0.8f, 1.0f);

        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();
        float r = 0.0f;
        float increment = 0.05f;

        while (!glfwWindowShouldClose(window))
        {
            glClear(GL_COLOR_BUFFER_BIT);

            shader.Bind();
            shader.SetUniform4f("u_Color", r, 0.3f, 0.8f, 1.0f);

            GLCall(glBindVertexArray(vao));
            va.Bind();
            ib.Bind();

            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            if (r > 1.0f)
                increment = -0.05f;
            else if (r < 0.0f)
                increment = 0.05f;

            r += increment;

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    // 清理资源，关闭 GLFW
    glfwTerminate();
    return 0;
}
