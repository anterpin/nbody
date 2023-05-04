#include "./shader.h"
#include <GL/glew.h>

class Interaction {
  ShaderProgram program;

public:
  Interaction() {
    program.parse("./shaders/inter.comp");
    program.link();
  }
  void set_eps(float eps) const {
    glProgramUniform1f(program.get_id(), 3, eps);
  }
  void set_damping(float damping) const {
    glProgramUniform1f(program.get_id(), 4, damping);
  }
  void set_G(float G) const { glProgramUniform1f(program.get_id(), 1, G); }
  void set_dt(float dt) const { glProgramUniform1f(program.get_id(), 2, dt); }
  void compute(size_t n) {
    program.use();
    glDispatchCompute(n / 256 + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }
};
