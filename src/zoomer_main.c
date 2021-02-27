#include <stdio.h>

#include "./zoomer_config.h"
#include "./zoomer_vector.h"
#include "./zoomer_navigation.h"

#define INITIAL_FL_DELTA_RADIUS 250.0f
#define FL_DELTA_RADIUS_DECELERATION 10.0f
#define PAN_SPEED 20.0f


typedef struct {
    bool is_enabled;
    float shadow;
    float radius;
    float delta_radius;
} Flashlight;

void flashlight_update(Flashlight *flashlight, const float dt) {
    if (fabs(flashlight->delta_radius) > 1.0f) {
        flashlight->radius = fmaxf(0.0f, flashlight->radius + flashlight->delta_radius * dt);
        flashlight->delta_radius -= flashlight->delta_radius * FL_DELTA_RADIUS_DECELERATION * dt;
    }

    if (flashlight->is_enabled) {
        flashlight->shadow = fminf(flashlight->shadow + 6.0f * dt, 0.8f);
    } else {
        flashlight->shadow = fmaxf(flashlight->shadow - 6.0f * dt, 0.0f);
    }
}

void draw(Vector image_size, Camera camera, GLuint shader_program, GLuint vertex_array_object, Vector window_size, Mouse mouse, Flashlight flashlight) {

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader_program);

    glUniform2f(glGetUniformLocation(shader_program, "cameraPos"), camera.position.x, camera.position.y);
    glUniform1f(glGetUniformLocation(shader_program, "cameraScale"), camera.scale);
    glUniform2f(glGetUniformLocation(shader_program, "screenshotSize"),
                image_size.x,
                image_size.y);
    glUniform2f(glGetUniformLocation(shader_program, "windowSize"),
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

void getCursorPosition(Mouse *mouse) {
    GetCursorPos(&mouse->point);
    mouse->curr.x = (float) mouse->point.x;
    mouse->curr.y = (float) mouse->point.y;
}

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

const char *vertex_shader_source = "#version 130\n" // 330 core
    "in vec3 aPos;\n"
    "in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "\n"
    "uniform vec2 cameraPos;\n"
    "uniform float cameraScale;\n"
    "uniform vec2 windowSize;\n"
    "uniform vec2 screenshotSize;\n"
    "uniform vec2 cursorPos;\n"
    "\n"
    "vec3 to_world(vec3 v) {\n"
    "    vec2 ratio = vec2(\n"
    "        windowSize.x / screenshotSize.x / cameraScale,\n"
    "        windowSize.y / screenshotSize.y / cameraScale);\n"
    "    return vec3((v.x / screenshotSize.x * 2.0 - 1.0) / ratio.x,\n"
    "                (v.y / screenshotSize.y * 2.0 - 1.0) / ratio.y,\n"
    "                v.z);\n"
    "}\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(to_world((aPos - vec3(cameraPos * vec2(1.0, -1.0), 0.0))), 1.0);\n"
    "   TexCoord = aTexCoord;\n"
    "}\n\0";

const char *fragment_shader_source = "#version 130\n" // 330 core
    "in mediump vec2 TexCoord;\n"
    "out mediump vec4 color;\n"
    "uniform sampler2D screenshot_texture;\n"
    "uniform vec2 cursorPos;\n"
    "uniform vec2 windowSize;\n"
    "uniform float flShadow;\n"
    "uniform float flRadius;\n"
    "uniform float cameraScale;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   vec4 cursor = vec4(cursorPos.x, windowSize.y - cursorPos.y, 0.0, 1.0);\n"
    "   color = mix(\n"
    "       texture(screenshot_texture, TexCoord), vec4(0.0, 0.0, 0.0, 0.0),\n"
    "       length(cursor - gl_FragCoord) < (flRadius * cameraScale) ? 0.0 : flShadow);\n"
    "}\n\0";

// TODO: No way to load config from file
Config config = { 0.01f, 1.5f, 6.0f, 4.0f };

// TODO: Handle user input without global variables
Mouse mouse = {0};
bool quitting = false;
bool control_key = false;
Camera camera = { .scale = 1.0f };
Flashlight flashlight = { .is_enabled = false, .radius = 200.0f };
int refreshRate = 60;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    (void) window;
    (void) scancode;
    (void) mods;
    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_Q:
        case GLFW_KEY_ESCAPE:
            quitting = true;
        break;
        case GLFW_KEY_EQUAL:
        case GLFW_KEY_KP_ADD:
            camera.scale *= 1.1f;
        break;
        case GLFW_KEY_MINUS:
        case GLFW_KEY_KP_SUBTRACT:
            camera.scale *= 0.90909f;
        break;
        case GLFW_KEY_KP_0:
        case GLFW_KEY_0:
            camera.scale = 1.0f;
            camera.delta_scale = 0.0f;
            camera.position = (Vector){ 0.0f, 0.0f };
            camera.velocity = (Vector){ 0.0f, 0.0f };
        break;
        case GLFW_KEY_W:
        case GLFW_KEY_UP:
            camera.position.y -= PAN_SPEED;
        break;
        case GLFW_KEY_S:
        case GLFW_KEY_DOWN:
            camera.position.y += PAN_SPEED;
        break;
        case GLFW_KEY_A:
        case GLFW_KEY_LEFT:
            camera.position.x -= PAN_SPEED;
        break;
        case GLFW_KEY_D:
        case GLFW_KEY_RIGHT:
            camera.position.x += PAN_SPEED;
        break;
        case GLFW_KEY_F:
            flashlight.is_enabled = !flashlight.is_enabled;
        break;
        case GLFW_KEY_LEFT_CONTROL:
        case GLFW_KEY_RIGHT_CONTROL:
            control_key = true;
        break;
        }
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
            control_key = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    (void) window;
    (void) xpos;
    (void) ypos;

    getCursorPosition(&mouse);
    if (mouse.drag) {
        Vector delta = VectorSubtract(world(camera, mouse.prev), world(camera, mouse.curr));
        VectorAddMut(&camera.position, delta);
        // delta is the distance the mouse traveled in a single
        // frame. To turn the velocity into units/second we need to
        // multiple it by FPS.
        camera.velocity = VectorScale(delta, (float) refreshRate);
    }
    mouse.prev = mouse.curr;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    (void) window;
    (void) mods;
    if (action == GLFW_PRESS) {
        if (button == GLFW_MOUSE_BUTTON_1) {
            mouse.prev = mouse.curr;
            mouse.drag = true;
            camera.velocity = (Vector){ 0.0f, 0.0f };
        }
    } else if (action == GLFW_RELEASE) {
        mouse.drag = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    (void) window;
    (void) xoffset;
    if (yoffset < 0) {
        if (control_key && flashlight.is_enabled) {
            flashlight.delta_radius += INITIAL_FL_DELTA_RADIUS;
        } else {
            camera.delta_scale += config.scroll_speed;
            camera.scale_pivot = mouse.curr;
        }
    } else {
        if (control_key && flashlight.is_enabled) {
            flashlight.delta_radius -= INITIAL_FL_DELTA_RADIUS;
        } else {
            camera.delta_scale -= config.scroll_speed;
            camera.scale_pivot = mouse.curr;
        }
    }
}
INT WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    (void) hInstance;
    (void) hPrevInstance;
    (void) lpCmdLine;
    (void) nCmdShow;

    // TODO: No support for non-fullscreen launch
    // TODO: No delay support

    SetProcessDPIAware();

    // Stolen from https://stackoverflow.com/a/28248531
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
    if(!lpPixels) {
        printf("%s\n", "failed to load texture from lpPixels");
        exit(1);
    }



    if (!glfwInit()) {
        fprintf(stderr, "Could not initialize GLFW!\n");
        exit(1);
    }
    GLFWmonitor* primary_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary_monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
     
    GLFWwindow *window = glfwCreateWindow(
        mode->width, mode->height,
        "Zoomer", primary_monitor, NULL);
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
        // positions                   // texture coords
        (float) w, (float) h, 0.0f,   1.0f, 1.0f, // top right
        (float) w, 0.0f,      0.0f,   1.0f, 0.0f, // bottom right
        0.0f,      0.0f,      0.0f,   0.0f, 0.0f, // bottom left
        0.0f,      (float) h, 0.0f,   0.0f, 1.0f  // top left
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, lpPixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    free(lpPixels);

    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "screenshot_texture"), 0);
    glEnable(GL_TEXTURE_2D);


    GLFWcursor *cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfwSetCursor(window, cursor);

    getCursorPosition(&mouse);
    mouse.prev = mouse.curr;

    Vector image_size = {(float) w, (float) h};
    Vector window_size = {(float) w, (float) h};

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    refreshRate = mode->refreshRate;
    float dt = 1.0f / mode->refreshRate;

    while (!glfwWindowShouldClose(window) && !quitting) {
        glViewport(0, 0, w, h);

        camera_update(&camera, config, dt, mouse, window_size);
        flashlight_update(&flashlight, dt);

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
