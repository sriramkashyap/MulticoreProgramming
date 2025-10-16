#include "image.h"

#include <opencv2/opencv.hpp> // Include the main OpenCV header

namespace image {

Image::Image(int width, int height) {
  image_ = cv::Mat::zeros(height, width, CV_8UC3); // A black RGB image
}
bool Image::SaveToFile(const std::string& path) {
  return cv::imwrite(path, image_);
}

void Image::Set(int x, int y, const RGB& value){
  image_.at<cv::Vec3b>(x, y) = cv::Vec3b(value.r, value.g, value.b);
}

RGB Image::Get(int x, int y){
  auto& pix = image_.at<cv::Vec3b>(x, y);
  return RGB(pix[0],pix[1],pix[2]);
}

void Image::Show(const std::string& window_name, int duration_ms) {
  cv::imshow(window_name.c_str(), image_);
}

Window::Window(const std::string& name) : name(name) {
  cv::namedWindow(name.c_str());
}

Window::~Window() {
  cv::destroyWindow(name.c_str());
}

void Window::WaitForMs(int value) {
  cv::waitKey(value);
}

} // namespace image.