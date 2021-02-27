#include <stdio.h>

#include "./zoomer_config.h"
#include "./zoomer_vector.h"
#include "./zoomer_navigation.h"


typedef struct {
    bool is_enabled;
    float shadow;
    float radius;
    float delta_radius;
} Flashlight;

const char *shader_type_as_cstr(GLenum shader_type)
{
    switch (shader_type) {
    case GL_VERTEX_SHADER:   return "GL_VERTEX_SHADER";
    case GL_GEOMETRY_SHADER: return "GL_GEOMETRY_SHADER";
    case GL_FRAGMENT_SHADER: return "GL_FRAGMENT_SHADER";
    default:                 return "<unknown>";
    }
}

GLuint compile_shader(const char *source_code, GLenum shader_type)
{
    GLuint shader = 0;
    shader = glCreateShader(shader_type);
    if (shader == 0) {
        fprintf(stderr, "Could not create a shader %s",
                shader_type_as_cstr(shader_type));
        exit(1);
    }

    glShaderSource(shader, 1, &source_code, 0);
    glCompileShader(shader);

    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLchar buffer[1024];
        int length = 0;
        glGetShaderInfoLog(shader, sizeof(buffer), &length, buffer);
        fprintf(stderr, "Could not compile shader %s: %s\n",
                shader_type_as_cstr(shader_type), buffer);
        exit(1);
    }

    return shader;
}

GLuint link_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLuint program = glCreateProgram();

    if (program == 0) {
        fprintf(stderr, "Could not create shader program");
        exit(1);
    }

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLchar buffer[1024];
        int length = 0;
        glGetProgramInfoLog(program, sizeof(buffer), &length, buffer);
        fprintf(stderr, "Could not link the program: %s\n", buffer);
        exit(1);
    }

    return program;
}

const char *vertex_shader_source = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "\n"
    "out vec3 ourColor;\n"
    "out vec2 TexCoord;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}\0";
const char *fragment_shader_source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "\n"
    "in vec3 ourColor;\n"
    "in vec2 TexCoord;\n"
    "\n"
    "uniform sampler2D screenshot_texture;"
    "\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture(screenshot_texture, TexCoord);\n"
    "}\n\0";

void draw(Vector image_size, Camera camera, GLuint shader_program, GLuint vertex_array_object, Vector window_size, Mouse mouse, Flashlight flashlight) {

    (void) image_size;
    (void) window_size;
    (void) mouse;
    (void) flashlight;
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT); // || GL_DEPTH_BUFFER_BIT
    glUseProgram(shader_program);

    glUniform2f(glGetUniformLocation(shader_program, "cameraPos"), camera.position.x, camera.position.y);
    glUniform1f(glGetUniformLocation(shader_program, "cameraScale"), camera.scale);
    glUniform2f(glGetUniformLocation(shader_program, "screenshotSize"),
                image_size.x,
                image_size.y);
    glUniform2f(glGetUniformLocation(shader_program, "window_size"),
                window_size.x,
                window_size.y);
    glUniform2f(glGetUniformLocation(shader_program, "cursorPos"),
                mouse.curr.x,
                mouse.curr.y);
    glUniform1f(glGetUniformLocation(shader_program, "flShadow"), flashlight.shadow);
    glUniform1f(glGetUniformLocation(shader_program, "flRadius"), flashlight.radius);

    glBindVertexArray(vertex_array_object);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

int main(int argc, char const *argv[])
{
    (void) argc;
    (void) argv;

    SetProcessDPIAware();

    // https://stackoverflow.com/a/28248531
    int x1, y1, x2, y2, w, h;

    // get screen dimensions
    x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
    x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    w   = x2 - x1;
    h   = y2 - y1;

    // copy screen to bitmap
    HDC     hScreen = GetDC(NULL);
    HDC     hDC     = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize GLFW!\n");
        exit(1);
    }

    GLFWwindow *window = glfwCreateWindow(
        w, h,
        "Zoomer", NULL, NULL);
    if (window == NULL) {
        glfwTerminate();
        fprintf(stderr, "Could not create GLFW window!\n");
        exit(1);
    }

    glfwMakeContextCurrent(window);

    if (GLEW_OK != glewInit()) {
        fprintf(stderr, "Could not initialize GLEW!\n");
        exit(1);
    }

    GLuint vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);
    GLuint shader_program = link_program(vertex_shader, fragment_shader);

    float vertices[] = {
        // positions          // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left
    };
    unsigned int indices[] = {
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    unsigned int vertex_buffer_object, vertex_array_object, element_buffer_object;
    glGenVertexArrays(1, &vertex_array_object);
    glGenBuffers(1, &vertex_buffer_object);
    glGenBuffers(1, &element_buffer_object);

    glBindVertexArray(vertex_array_object);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);


    GLuint screenshot_texture;

    glGenTextures(1, &screenshot_texture);
    glBindTexture(GL_TEXTURE_2D, screenshot_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    BITMAP bitmap;
    GetObject(hBitmap, sizeof(bitmap), (void *) &bitmap);
    BITMAPINFO BMInfo = {0};
    BMInfo.bmiHeader.biSize = sizeof(BMInfo.bmiHeader);
    if(0 == GetDIBits(hDC, hBitmap, 0, 0, NULL, &BMInfo, DIB_RGB_COLORS))
    {
        printf("%s\n", "couldn't get DIBits to BMInfo");
    }
    BYTE* lpPixels = (BYTE*) malloc(BMInfo.bmiHeader.biSizeImage);
    if (lpPixels == 0) {
        printf("ERROR: Out of memory\n");
        return 1;
    }

    BMInfo.bmiHeader.biBitCount = 32;
    BMInfo.bmiHeader.biCompression = BI_RGB;
    BMInfo.bmiHeader.biHeight = abs(BMInfo.bmiHeader.biHeight);
    if(0 == GetDIBits(hDC, hBitmap, 0, BMInfo.bmiHeader.biHeight,
              lpPixels, &BMInfo, DIB_RGB_COLORS)) {
        printf("%s\n", "couldn't get DIBits to lpPixels");
    }
    if (lpPixels) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, lpPixels);
        glGenerateMipmap(GL_TEXTURE_2D);
        free(lpPixels);
    } else {
        printf("%s\n", "failed to load texture from lpPixels");

    }

    glUseProgram(shader_program);

    Camera camera = {0};
    Mouse mouse = {0};
    Flashlight flashlight = {0};



    glUniform1i(glGetUniformLocation(shader_program, "screenshot_texture"), 0);


    Vector image_size = {(float) w, (float) h};
    Vector window_size = {(float) w, (float) h};
    while (!glfwWindowShouldClose(window)) {
        draw(image_size, camera, shader_program, vertex_array_object, window_size, mouse, flashlight);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);
    return 0;
}
