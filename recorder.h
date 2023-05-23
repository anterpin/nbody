#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string>

class Recorder {
  FILE *ffmpeg = nullptr;

public:
  int w, h;
  Recorder(int w, int h, const std::string &filename) : w{w}, h{h} {
    std::stringstream ss;
    ss << "ffmpeg -y -f rawvideo -s " << w << 'x' << h
       << " -pix_fmt rgb24 -r 25 -i - -vf vflip -an -b:v 1000k " << filename;
    ffmpeg = popen(ss.str().c_str(), "w");
    if (!ffmpeg) {
      std::cerr << "Cannot open " << ss.str() << '\n';
    }
  }
  size_t write(const void *pixels, size_t size) {
    if (ffmpeg) {
      fwrite(pixels, size, 1, ffmpeg);
    }
    return 0;
  }

  ~Recorder() {
    if (ffmpeg) {
      pclose(ffmpeg); // without checking res
      ffmpeg = nullptr;
    }
  }
};
