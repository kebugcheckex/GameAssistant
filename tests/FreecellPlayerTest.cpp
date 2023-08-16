#include "pch.h"

#include <gtest/gtest.h>
// #include <gmock/gmock.h>

#include "../FreecellPlayer.h"
#include "../IGameWindow.h"

namespace game_assistant {
namespace freecell {

// TODO Gmock has some weird errors, manually mock for now
class MockGameWindow : public IGameWindow {
 public:
  // MOCK_METHOD(cv::Mat, getSnapshot, (), (override));
  cv::Mat getSnapshot() override { return cv::Mat(); };
  void clickAt(int x, int y) override{};
  void pressKey(char ch) override{};
};

TEST(TestParseSolutionsFile, parseSuccessful) {
  auto mockGameWindow = std::make_shared<MockGameWindow>();
  FreecellPlayer player(mockGameWindow);
  player.play(R"(..\data\solve.txt)");
}
}  // namespace freecell
}  // namespace game_assistant
