// Copyright 2021 Tushar Maheshwari. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <algorithm> // for count
#include <array>     // for array
#include <cmath>     // for abs
#include <cstddef>   // for size_t
#include <memory>    // for shared_ptr
#include <random>    // for random_device, mt19937
#include <string>    // for to_string, string
#include <utility>   // for move, swap

#include <ftxui/component/component.hpp>          // for CatchEvent, Renderer
#include <ftxui/component/event.hpp>              // for Event
#include <ftxui/component/screen_interactive.hpp> // for ScreenInteractive
#include <ftxui/dom/elements.hpp> // for text, hbox, vbox, window, Element, Elements

template <typename Element, size_t Height, size_t Width = Height> class Grid {
public:
  constexpr static auto height = Height;
  constexpr static auto width = Width;
  constexpr static auto MaximumNumber = height * width;
  std::array<Element, MaximumNumber> elements{};
  std::mt19937 prng;

  Grid() : prng{std::random_device{}()} {
    insertOne();
  }

  size_t DigitsInMaximumNumber() const {
    auto number = 1 << *std::max_element(elements.begin(), elements.end());
    size_t ret = 1;
    while (number /= 10)
      ++ret;
    return ret;
  }

  void move(int x, int y) {
    assert(std::abs(x) + std::abs(y) == 1);
    const bool transpose = std::abs(y) == 1;
    const auto outer_end = transpose ? width : height;
    const auto direction = transpose ? y : x;
    const auto inner_begin = direction == 1 ? 0 : (height + width - outer_end) - 1;
    const auto inner_end = inner_begin + direction * (height + width - outer_end);
    for (int outer = 0; outer != outer_end; ++outer) {
      for (int inner = inner_begin, write = inner_begin; inner != inner_end; inner += direction) {
        if (at(outer, inner, transpose) != 0) {
          std::swap(at(outer, write, transpose), at(outer, inner, transpose));
          write += direction;
        }
      }
    }
    insertOne();
  }

private:
  Element& at(int row, int col, bool transpose) {
    return transpose ? elements[col * width + row] : elements[row * width + col];
  }

  void insertOne() {
    if (auto zeros = std::count(elements.begin(), elements.end(), 0)) {
      auto index = prng() % zeros;
      int counter = 0;
      for (auto& elem : elements) {
        if (elem == 0 && counter++ == index) {
          elem = 1;
          break;
        }
      }
    }
  }
};

using namespace ftxui;

int main(int argc, const char *argv[]) {
  auto screen = ScreenInteractive::FitComponent();
  using State = Grid<int, 4>;
  State state;

  auto component = Renderer([&state] {
    Elements children;
    Elements row;
    const auto DigitsInMaximumVisibleNumber = state.DigitsInMaximumNumber();
    for (auto cell : state.elements) {
      if (cell == 0) {
        row.push_back(text("") |
                      size(WIDTH, EQUAL, DigitsInMaximumVisibleNumber) |
                      border);
      } else {
        row.push_back(text(std::to_string(1 << cell)) | align_right |
                      size(WIDTH, EQUAL, DigitsInMaximumVisibleNumber) |
                      border);
      }
      if (row.size() == State::width) {
        children.push_back(hbox(std::move(row)));
        row.clear();
      }
    }
    return window(text("1 << 11"), vbox(std::move(children)));
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
