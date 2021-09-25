// Copyright 2021 Tushar Maheshwari. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <algorithm> // for find, shuffle
#include <array>     // for array
#include <cstddef>   // for size_t
#include <memory>    // for shared_ptr
#include <numeric>   // for iota
#include <random>    // for random_device, mt19937
#include <string>    // for to_string, string
#include <utility>   // for move, swap

#include <ftxui/component/component.hpp>          // for CatchEvent, Renderer
#include <ftxui/component/event.hpp>              // for Event
#include <ftxui/component/screen_interactive.hpp> // for ScreenInteractive
#include <ftxui/dom/elements.hpp> // for text, hbox, vbox, window, Element, Elements

constexpr static size_t DigitsInNumber(size_t number) {
  size_t ret = 1;
  while (number /= 10)
    ++ret;
  return ret;
}

template <typename Element, size_t Height, size_t Width = Height> struct Grid {
  constexpr static auto height = Height;
  constexpr static auto width = Width;
  constexpr static auto MaximumNumber = height * width;
  std::array<Element, MaximumNumber> elements;

  Grid() {
    std::iota(elements.begin(), elements.end(), 1);
    std::random_device rd;
    std::shuffle(elements.begin(), elements.end(), std::mt19937{rd()});

    // TODO: ensure solvability.
  }

  void move(int x, int y) {
    auto blankPos = std::find(elements.begin(), elements.end(), MaximumNumber) -
                    elements.begin();
    auto swapWithRow = blankPos / width + y;
    auto swapWithCol = blankPos % width + x;
    if (swapWithRow >= 0 && swapWithRow < height && swapWithCol >= 0 &&
        swapWithCol < width) {
      auto swapWithPos = swapWithRow * width + swapWithCol;
      std::swap(elements[blankPos], elements[swapWithPos]);
    }
  }
};

using namespace ftxui;

int main(int argc, const char *argv[]) {
  auto screen = ScreenInteractive::FitComponent();
  using State = Grid<int, 4>;
  State state;
  constexpr auto DigitsInMaximumVisibleNumber =
      DigitsInNumber(State::MaximumNumber - 1);

  auto component = Renderer([&state] {
    Elements children;
    Elements row;
    for (auto cell : state.elements) {
      if (cell == State::MaximumNumber) {
        row.push_back(text("") |
                      size(WIDTH, EQUAL, DigitsInMaximumVisibleNumber) |
                      inverted | border);
      } else {
        row.push_back(text(std::to_string(cell)) | align_right |
                      size(WIDTH, EQUAL, DigitsInMaximumVisibleNumber) |
                      border);
      }
      if (row.size() == State::width) {
        children.push_back(hbox(std::move(row)));
        row.clear();
      }
    }
    return window(text("Tiles"), vbox(std::move(children)));
  });

  component = CatchEvent(component, [&](Event event) {
    if (event == Event::Escape || event.input() == "q") {
      screen.ExitLoopClosure()();
    } else if (event == Event::Return || event.input() == "r") {
      state = {};
    } else if (event == Event::ArrowUp) {
      state.move(0, 1);
    } else if (event == Event::ArrowDown) {
      state.move(0, -1);
    } else if (event == Event::ArrowLeft) {
      state.move(1, 0);
    } else if (event == Event::ArrowRight) {
      state.move(-1, 0);
    }
    return false;
  });

  screen.Loop(component);
}
