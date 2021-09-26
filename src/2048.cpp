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
  size_t score{}, score_increase{};
  bool game_over = false;

  Grid() { insertOne(); }

  static std::string Stringify(Element elem) {
    return elem ? std::to_string(1 << elem) : "";
  }

  static size_t DigitsInMaximumNumber() {
    return Stringify(MaximumNumber).size();
  }

  void move(int x, int y) {
    assert(std::abs(x) + std::abs(y) == 1);
    bool valid = false;
    size_t move_increase = 0;
    const bool transpose = std::abs(y) == 1;
    const auto outer_end = transpose ? width : height;
    const auto direction = transpose ? y : x;
    const auto inner_begin =
        direction == 1 ? 0 : (height + width - outer_end) - 1;
    const auto inner_end =
        inner_begin + direction * (height + width - outer_end);
    for (size_t outer = 0; outer != outer_end; ++outer) {
      bool mergeable = false;
      auto write = inner_begin - direction;
      for (auto inner = inner_begin; inner != inner_end; inner += direction) {
        if (at(outer, inner, transpose) != 0) {
          if (mergeable &&
              at(outer, write, transpose) == at(outer, inner, transpose)) {
            mergeable = false;
            move_increase += 1 << ++at(outer, write, transpose);
            at(outer, inner, transpose) = 0;
            valid = true;
          } else {
            mergeable = true;
            write += direction;
            if (write != inner) {
              std::swap(at(outer, write, transpose),
                        at(outer, inner, transpose));
              valid = true;
            }
          }
        }
      }
    }
    if (valid) {
      insertOne();
      score += score_increase = move_increase;
    }
  }

private:
  std::mt19937 prng{std::random_device{}()};

  Element &at(int row, int col, bool transpose = false) {
    return transpose ? elements[col * width + row]
                     : elements[row * width + col];
  }

  void insertOne() {
    if (auto zeros = std::count(elements.begin(), elements.end(), 0)) {
      auto index = prng() % zeros;
      int counter = 0;
      for (auto &elem : elements) {
        if (elem == 0 && counter++ == index) {
          elem = prng() % 6 ? 1 : 2; // 16% chance for 4 tiles.
          break;
        }
      }
      if (zeros == 1) {
        for (size_t row = 0; row != height; ++row) {
          for (size_t col = 0; col != width; ++col) {
            if ((row && at(row, col) == at(row - 1, col)) ||
                (col && at(row, col) == at(row, col - 1))) {
              return;
            }
          }
        }
        game_over = true;
      }
    } else {
      game_over = true;
    }
  }
};

using namespace ftxui;

int main(int /* argc */, const char * /* argv */[]) {
  auto screen = ScreenInteractive::FitComponent();
  Grid<int, 4> state;

  auto component = Renderer([&state] {
    Elements children;
    Elements row;
    const auto DigitsInMaximumNumber = state.DigitsInMaximumNumber();
    for (auto cell : state.elements) {
      row.push_back(text(state.Stringify(cell)) | bold | center |
                    size(WIDTH, EQUAL, DigitsInMaximumNumber) | border |
                    color(Color{static_cast<Color::Palette16>(cell)}));
      if (row.size() == state.width) {
        children.push_back(hbox(std::move(row)));
        row.clear();
      }
    }
    if (state.game_over) {
      children.push_back(text("Game Over") | bold | center | border |
                         color(Color::Red));
    }
    children.push_back(separator());
    children.push_back(
        hbox(text("Score: "), text(std::to_string(state.score)) | bold,
             text("(+" + std::to_string(state.score_increase) + ")") |
                 color(Color::Green)));
    return window(text(" 1 << 11 "), vbox(std::move(children)));
  });

  component = CatchEvent(component, [&](Event event) {
    if (event == Event::Escape || event.input() == "q") {
      screen.ExitLoopClosure()();
    } else if (event == Event::Return || event.input() == "r") {
      state = {};
    } else if (event == Event::ArrowUp || event.input() == "w") {
      state.move(0, 1);
    } else if (event == Event::ArrowDown || event.input() == "s") {
      state.move(0, -1);
    } else if (event == Event::ArrowLeft || event.input() == "a") {
      state.move(1, 0);
    } else if (event == Event::ArrowRight || event.input() == "d") {
      state.move(-1, 0);
    }
    return false;
  });

  screen.Loop(component);
}
