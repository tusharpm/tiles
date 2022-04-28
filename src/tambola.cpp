// Copyright 2022 Tushar Maheshwari. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <algorithm> // for shuffle
#include <array>     // for array
#include <cstddef>   // for size_t
#include <iomanip>   // for setw
#include <iostream>  // for cout
#include <memory>    // for shared_ptr
#include <numeric>   // for iota
#include <random>    // for random_device, mt19937
#include <string>    // for to_string, string
#include <utility>   // for move, swap
#include <vector>    // for vector

struct Grid {
  using num_t = uint8_t;
  struct Ticket {
    constexpr static size_t height = 3;
    constexpr static size_t width = 9;
    num_t numbers[height][width] = {};

    size_t getRowCount(size_t r) const {
      size_t count = 0;
      for (auto num : numbers[r]) {
        if (num) {
          ++count;
        }
      }
      return count;
    }

    void sortColumns() {
      for (size_t c = 0; c != width; ++c) {
        std::array<num_t, height> col;
        auto iter = col.begin();
        for (auto& row : numbers) {
          if (row[c]) {
            *iter++ = row[c];
          }
        }
        std::sort(col.begin(), iter);
        iter = col.begin();
        for (auto& row : numbers) {
          if (row[c]) {
            row[c] = *iter++;
          }
        }
      }
    }
  };
  std::array<Ticket, 6> tickets;
  std::mt19937 prng{std::random_device{}()};

  template <typename T>
  static auto cardinality(const T& set) {
    size_t count = 0;
    for (const auto& li : set) {
      count += li.size();
    }
    return count;
  }

  static auto iotaRange(num_t from, num_t to) {
    std::vector<num_t> result;
    result.reserve(++to - from);
    for (; from != to; ++from) {
      result.push_back(from);
    }
    return result;
  }

  template <typename T>
  auto getRand(T& collection) {
    return collection.begin() + (prng() % collection.size());
  }

  template<size_t size>
  auto shuffledRange() {
    std::array<size_t, size> result;
    std::iota(result.begin(), result.end(), 0);
    std::shuffle(result.begin(), result.end(), prng);
    return result;
  }

  Grid() {
    std::array columns {
      iotaRange(1, 9),
      iotaRange(10, 19),
      iotaRange(20, 29),
      iotaRange(30, 39),
      iotaRange(40, 49),
      iotaRange(50, 59),
      iotaRange(60, 69),
      iotaRange(70, 79),
      iotaRange(80, 90),
    };

    std::array<decltype(columns), std::tuple_size_v<decltype(tickets)>> sets;

    // assigning elements to each set for each column
    for (size_t i = 0; i != columns.size(); ++i) {
      auto& col = columns[i];
      for (auto& set : sets) {
        auto randNumIter = getRand(col);
        set[i].push_back(*randNumIter);
        col.erase(randNumIter);
      }
    }

    // assign element from last column to random set
    {
      auto& lastCol = columns.back();
      getRand(sets)->back().push_back(lastCol.back());
      lastCol.pop_back();
    }

    // 3 + 1 passes over the remaining columns
    for (size_t pass = 0; pass < 4u; ++pass) {
      const size_t colLimit = 2 + pass / 3;
      for (size_t i = 0; i != columns.size(); ++i) {
        if (auto& col = columns[i]; !col.empty()) {
          for (auto randSetIndex : shuffledRange<sets.size()>()) {
            auto& randSet = sets[randSetIndex];
            if (randSet[i].size() != colLimit && cardinality(randSet) != 15) {
              randSet[i].push_back(col.back());
              col.pop_back();
              break;
            }
          }
        }
      }
    }

    // got the sets - need to arrange in tickets now
    for (size_t setIndex = 0; setIndex != sets.size(); ++setIndex) {
      auto& currSet = sets[setIndex];
      auto& currTicket = tickets[setIndex];

      // fill each row
      for (size_t row = 0; row != Ticket::height; ++row) {
        for (size_t size = Ticket::height - row; size != 0; --size) {
          for (auto col : shuffledRange<columns.size()>()) {
            if (currTicket.numbers[row][col] == 0) {
              if (auto& currSetCol = currSet[col]; currSetCol.size() == size) {
                currTicket.numbers[row][col] = currSetCol.back();
                currSetCol.pop_back();
                if (currTicket.getRowCount(row) == 5) {
                  size = 1; // break nested size loop
                  break;
                }
              }
            }
          }
        }
      }

      // quick patch to ensure columns are sorted
      currTicket.sortColumns();
    }
  }
};

int main() {
  Grid g;

  // print the tickets
  bool first = true;
  for (auto& ticket : g.tickets) {
    if (first) {
      first = false;
    } else {
      std::cout << "\n\n";
    }
    for (size_t r = 0; r != ticket.height; ++r) {
      if (r) {
        std::cout << '\n';
      }
      for (size_t c = 0; c != ticket.width; ++c) {
        if (c) {
          std::cout << " | ";
        }
        if (unsigned num = ticket.numbers[r][c]) {
          std::cout << std::setw(2) << num;
        } else {
          std::cout << "  ";
        }
      }
    }
  }

  std::cout << std::endl;
}
