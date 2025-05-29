//
// Created by Jason on 2025/5/27.
//

#include "PrimitiveDrawer.h"
#include <GLES3/gl3.h>
#include <cmath>
#include <android/log.h>

// 顶点着色器和片段着色器的源代码
const char* vertexShaderSource = R"(#version 300 es
layout (location = 0) in vec2 aPos;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(#version 300 es
precision mediump float;
out vec4 FragColor;
uniform vec4 uColor;
void main() {
    FragColor = uColor;
}
)";

// 编译着色器的辅助函数
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    // 检查编译错误
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        __android_log_print(ANDROID_LOG_ERROR, "SmartzMap", "Shader compilation failed: %s", infoLog);
        return 0;
    }
    return shader;
}

// OpenGL程序ID
GLuint shaderProgram = 0;

void useProgram() {
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f); // 设置背景色
    glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区

    if (shaderProgram == 0) {
        // 编译着色器
        GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        // 创建和链接着色器程序
        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        // 检查链接错误
        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
            __android_log_print(ANDROID_LOG_ERROR, "SmartzMap", "Program linking failed: %s", infoLog);
            return;
        }

        // 清理着色器对象
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    // 使用着色器程序
    glUseProgram(shaderProgram);
}

// 绘制一个圆形
// 参数：圆心坐标 (x, y)，半径 radius，颜色 color（ARGB 格式）
// 注意：此函数假设 OpenGL 上下文已正确设置，并且使用了适当的着色器程序
void drawCircle(float x, float y, float radius, int color) {
    const int segments = 64;  // 圆的精细度
    const float PI = 3.14159265359f;

    useProgram();

    // 创建顶点数组
    float vertices[(segments + 2) * 2];

    // 圆心
    vertices[0] = x;
    vertices[1] = y;

    // 生成圆周上的顶点
    for (int i = 0; i <= segments; i++) {
        float angle = i * 2.0f * PI / segments;
        vertices[(i + 1) * 2] = x + radius * cos(angle);
        vertices[(i + 1) * 2 + 1] = y + radius * sin(angle);
    }

    // 创建并绑定顶点缓冲对象
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 启用顶点属性数组
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    // 绘制三角形扇形
    glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

    // 清理
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &vbo);
}

void drawLine(float x1, float y1, float x2, float y2, int color) {
    useProgram();

    // 创建顶点数组
    float vertices[] = {
        x1, y1,  // 起点
        x2, y2   // 终点
    };

    // 从颜色值中提取RGBA分量
    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = (color & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;

    // 设置线条颜色
    GLint colorLoc = glGetUniformLocation(shaderProgram, "uColor");
    glUniform4f(colorLoc, r, g, b, a);

    // 创建并绑定顶点缓冲对象
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 设置顶点属性
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    // 设置线宽（注意：在OpenGL ES中，线宽可能被限制为1.0）
    glLineWidth(12.0f);

    // 绘制直线
    glDrawArrays(GL_LINES, 0, 2);

    // 清理资源
    glDisableVertexAttribArray(0);
    glDeleteBuffers(1, &vbo);
}
