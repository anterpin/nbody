#pragma once
#include "./utils.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <iostream>
#include <vector>

class Input {
public:
  char mouse_buttons[10] = {0};
  double scroll[2] = {0};
  double mouse_pos[2] = {0};
  double d_mouse_pos[2] = {0};
  char keys[350] = {0};
};

static Input input;

static void input_reset() {
  input.d_mouse_pos[0] = 0;
  input.d_mouse_pos[1] = 0;
  input.scroll[0] = 0;
  input.scroll[1] = 0;
}

static void key_callback(GLFWwindow *win, int key, int scancode, int action,
                         int mode) {
  if (key == GLFW_KEY_UNKNOWN)
    return;
  if (key)
    input.keys[key] = action;
}

static void cursor_position_callback(GLFWwindow *window, double xpos,
                                     double ypos) {
  input.d_mouse_pos[0] = xpos - input.mouse_pos[0];
  input.d_mouse_pos[1] = ypos - input.mouse_pos[1];

  input.mouse_pos[0] = xpos;
  input.mouse_pos[1] = ypos;
}

static void cursor_enter_callback(GLFWwindow *window, int entered) {
  if (!entered) {
    /* memset(input.keys, GLFW_RELEASE, sizeof(input.keys)); */
    /* memset(input.mouse_buttons, GLFW_RELEASE, sizeof(input.mouse_buttons));
     */
  }
}

static void mouse_button_callback(GLFWwindow *window, int button, int action,
                                  int mods) {
  input.mouse_buttons[button] = action;
}

static void scroll_callback(GLFWwindow *window, double xoffset,
                            double yoffset) {
  input.scroll[0] = xoffset;
  input.scroll[1] = yoffset;
  /* std::cout << xoffset << ' ' << yoffset << "\n"; */
}

class App {
  GLFWwindow *window;

  static constexpr glm::vec3 color{0, 0, 0};

  int w, h;

public:
  void handle(Input *input) {
    if (input->keys[GLFW_KEY_ESCAPE]) {
      glfwSetWindowShouldClose(window, GL_TRUE);
      return;
    }
  }

  App(int _w, int _h, const char *title) : h{_h}, w{_w} {
    window = glfwCreateWindow(w, h, title, NULL, NULL);
    check(window, "Cannot create a window");
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, 1);
    /* glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, 1); */
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetCursorEnterCallback(window, cursor_enter_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glewExperimental = GL_TRUE;
    check(glewInit() == GLEW_OK, "Cannot initlialize glew");

    glViewport(0, 0, w, h);
    glPointSize(2);
  }
  App(const App &) = delete;
  App operator=(const App &) = delete;

  void set_resize_callback(void (*callback)(GLFWwindow *, int, int)) const {
    glfwSetFramebufferSizeCallback(window, callback);
  }

  void set_dimensions(int _w, int _h) {
    w = _w;
    h = _h;
    glViewport(0, 0, w, h);
  }
  bool should_close() const {
    glfwSwapBuffers(window);
    if (input.keys[GLFW_KEY_ESCAPE] == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, true);
    }
    input_reset();
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(color.x, color.y, color.z, 1.0);
    return glfwWindowShouldClose(window);
  }

  ~App() { glfwDestroyWindow(window); }
};

class Init {
  static void error_callback(int err, const char *desc) {
    std::cerr << err << ' ' << desc << '\n';
  }

public:
  Init() {
    check(glfwInit() == GLFW_TRUE, "Cannot initialize the window");
    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    /* glfwWindowHint(GLFW_RESIZABLE, GL_FALSE); */
  }
  Init(const Init &) = delete;
  Init operator=(const Init &) = delete;
  ~Init() { glfwTerminate(); }
};
