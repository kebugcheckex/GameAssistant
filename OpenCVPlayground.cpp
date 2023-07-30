#include "pch.h"
#include "OpenCVPlayground.h"

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

void OpenCVPlayground::blueChannel() {
  cv::Mat image = cv::imread("./images/ice3.png");

  if (image.empty()) {
    std::cerr << "Error: Image not found or unable to read." << std::endl;
    return;
  }

  // Split the image into its individual color channels (B, G, R)
  std::vector<cv::Mat> channels;
  cv::split(image, channels);

  for (int i = 0; i < 3; i++) {
    cv::imshow("Channels", channels[i]);
    cv::waitKey(0);
  }

  cv::destroyAllWindows();
}

void OpenCVPlayground::templateMatching() {
  cv::Mat largerImage = cv::imread("./images/icebreak.png");

  for (int i = 1; i <= 3; i++) {
    std::stringstream ss;
    ss << "./images/ice" << i << ".png";
    std::cout << "Matching image " << ss.str();
    cv::Mat templateImage = cv::imread(ss.str());

    if (largerImage.empty() || templateImage.empty()) {
      std::cerr << "Error: Image not found or unable to read." << std::endl;
      return;
    }

    // Perform template matching using the method of your choice
    cv::Mat result;
    int matchMethod = cv::TM_CCOEFF_NORMED;  // You can try other methods like
                                             // TM_SQDIFF, TM_CCORR, etc.
    cv::matchTemplate(largerImage, templateImage, result, matchMethod);
    std::cout << "After template match\n";

    // Define a threshold for template matching results
    double threshold = 0.9;  // Adjust this value based on your use case

    // Find template matching locations above the threshold
    std::vector<cv::Point> locations;
    cv::findNonZero(result > threshold, locations);
    if (locations.empty()) {
      std::cout << "Cannot find any matches\n";
      return;
    }

    // Draw rectangles around the matched regions
    for (const cv::Point& point : locations) {
      cv::rectangle(
          largerImage, point,
          cv::Point(point.x + templateImage.cols, point.y + templateImage.rows),
          cv::Scalar(0, 0, 255), 2);

      int x = point.x / 96;
      int y = point.y / 96;
      std::stringstream ss1;
      ss1 << "(" << x << ", " << y << ", " << i << ")";
      cv::putText(largerImage, ss1.str(),
          cv::Point(point.x + templateImage.cols + 10, point.y + templateImage.rows),
                  cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(200, 0, 200), 2);
    }

  }

  // Display the result with rectangles around the matched regions
  cv::imshow("Template Matching Result", largerImage);
  cv::waitKey(0);
  cv::destroyAllWindows();
}