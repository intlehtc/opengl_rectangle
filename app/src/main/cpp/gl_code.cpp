/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

const char gVertexShader[] =
        "#version 300 es                            \n"
                "layout(location = 0) in vec4 a_position;   \n"
                "layout(location = 1)  in  vec4 a_color;      \n"
                " out  vec4 v_color;                          \n"
                "void main()                                \n"
                "{                                          \n"
                "    v_color = a_color;                     \n"
                "    gl_Position = a_position;              \n"
                "}";

const char  gFragmentShader[] =
        "#version 300 es            \n"
                "precision mediump float;   \n"
                " in  vec4 v_color;           \n"
               " out  vec4 o_fragColor;      \n"
                "void main()                \n"
                "{                          \n"
                "    o_fragColor = v_color; \n"
                "}" ;

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
//GLuint gvPositionHandle;

bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }

    //获取属性变量vPosition在着色器程序中的地址
  //  gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
 //   checkGlError("glGetAttribLocation");
 //   LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
   //         gvPositionHandle);

    glViewport(0, 0, w, h); //调用glViewPort函数来决定视见区域，告诉OpenGL应把渲染之后的图形绘制在窗体的哪个部位
    checkGlError("glViewport");
    return true;
}
/*
const GLfloat gTriangleVertices[] = { 0.0f, 0.5f, -0.5f, -0.5f,
        0.5f, -0.5f };*/
const GLfloat gTriangleVertices[] =
        {
                -0.5, -0.5, 0,  //左下
                0.5, -0.5, 0,  //右下
                -0.5,  0.5, 0,  //左上
                0.5,  0.5, 0,  //右上
        };

//const GLfloat color[4] = { 1.0f, 0.0f, 0.0f, 1.0f };

//4个点的颜色(分别表示RGBA值)
const GLfloat gColor[] = {

        1,1,1,1,
        0,1,0,1,
        0,0,1,1,
        0,0,0,1,
};

void renderFrame() {
    static float grey;
    grey += 0.01f;
    if (grey > 1.0f) {
        grey = 0.0f;
    }
    // glClearColor和glClear必须成对出现，前者是设置清除颜色，后者是执行清除
    glClearColor(grey, grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    /*，由glEnableVertexAttribArray启用指定属性，才可在顶点着色器中访问逐顶点的属性数据。
     * glVertexAttribPointer只是建立CPU和GPU之间的逻辑连接，从而实现了CPU数据上传至GPU。
     * 但是，数据在GPU端是否可见，即，着色器能否读取到数据，由是否启用了对应的属性决定，这就是glEnableVertexAttribArray的功能，允许顶点着色器读取GPU（服务器端）数据。
*/
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);

    glEnableVertexAttribArray(0);
    checkGlError("glEnableVertexAttribArray");

   glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, gColor);

    glEnableVertexAttribArray(1);

  // glVertexAttrib4fv ( 1, color );

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);    // 参数3：顶点数量  参数2：从哪个顶点开始绘制
    checkGlError("glDrawArrays");
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_step(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_android_gl2jni_GL2JNILib_step(JNIEnv * env, jobject obj)
{
    renderFrame();
}
