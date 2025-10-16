#ifndef IMAGE_H
#define IMAGE_H

#include<string>
#include <opencv2/opencv.hpp>

namespace image {

struct RGB{
  char r,g,b;
  RGB(char r, char g, char b) : r(r), g(g), b(b){}
};

class Image {
public:
  explicit Image(int width, int height);
  bool SaveToFile(const std::string& path);
  void Set(int x, int y, const RGB& value);
  RGB Get(int x, int y);
  void Show(const std::string& window_name, int duration_ms = 10);
  
protected:
  cv::Mat image_;
};

class Window {
public:
  explicit Window(const std::string& name);
  ~Window();
  void WaitForMs(int value);
protected:
  std::string name;
};
  
} // namespace image.

#endif // IMAGE_H