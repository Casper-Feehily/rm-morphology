#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

class BinaryImage {
public:
  BinaryImage(std::size_t rows, std::size_t cols, std::vector<int> pixels)
  : rows_(rows), cols_(cols), pixels_(std::move(pixels))
  {
    if (rows == 0 || cols == 0 || pixels_.size() != rows * cols) {
      throw std::invalid_argument("image dimensions do not match pixel count");
    }
    for (int pixel : pixels_) {
      if (pixel != 0 && pixel != 1) {
        throw std::invalid_argument("binary image pixels must be 0 or 1");
      }
    }
  }

  int at(std::size_t row, std::size_t col) const { return pixels_[row * cols_ + col]; }
  std::size_t rows() const { return rows_; }
  std::size_t cols() const { return cols_; }

private:
  std::size_t rows_;
  std::size_t cols_;
  std::vector<int> pixels_;
};

class StructuringElement {
public:
  StructuringElement(std::size_t rows, std::size_t cols, std::vector<int> mask)
  : rows_(rows), cols_(cols), mask_(std::move(mask))
  {
    if (rows == 0 || cols == 0 || rows % 2 == 0 || cols % 2 == 0 || mask_.size() != rows * cols) {
      throw std::invalid_argument("structuring element must have odd dimensions");
    }
    for (int value : mask_) {
      if (value != 0 && value != 1) {
        throw std::invalid_argument("structuring element values must be 0 or 1");
      }
    }
  }

  int at(std::size_t row, std::size_t col) const { return mask_[row * cols_ + col]; }
  std::size_t rows() const { return rows_; }
  std::size_t cols() const { return cols_; }

private:
  std::size_t rows_;
  std::size_t cols_;
  std::vector<int> mask_;
};

BinaryImage dilate(const BinaryImage & image, const StructuringElement & element)
{
  std::vector<int> output(image.rows() * image.cols(), 0);
  const auto row_radius = static_cast<std::ptrdiff_t>(element.rows() / 2);
  const auto col_radius = static_cast<std::ptrdiff_t>(element.cols() / 2);

  for (std::size_t row = 0; row < image.rows(); ++row) {
    for (std::size_t col = 0; col < image.cols(); ++col) {
      for (std::size_t erow = 0; erow < element.rows() && output[row * image.cols() + col] == 0; ++erow) {
        for (std::size_t ecol = 0; ecol < element.cols(); ++ecol) {
          const auto image_row = static_cast<std::ptrdiff_t>(row) + static_cast<std::ptrdiff_t>(erow) - row_radius;
          const auto image_col = static_cast<std::ptrdiff_t>(col) + static_cast<std::ptrdiff_t>(ecol) - col_radius;
          if (element.at(erow, ecol) == 1 && image_row >= 0 && image_col >= 0 &&
              image_row < static_cast<std::ptrdiff_t>(image.rows()) &&
              image_col < static_cast<std::ptrdiff_t>(image.cols()) &&
              image.at(static_cast<std::size_t>(image_row), static_cast<std::size_t>(image_col)) == 1) {
            output[row * image.cols() + col] = 1;
            break;
          }
        }
      }
    }
  }
  return BinaryImage(image.rows(), image.cols(), std::move(output));
}

void print(const BinaryImage & image)
{
  for (std::size_t row = 0; row < image.rows(); ++row) {
    for (std::size_t col = 0; col < image.cols(); ++col) {
      std::cout << image.at(row, col) << (col + 1 == image.cols() ? '\n' : ' ');
    }
  }
}

int main()
{
  const BinaryImage input(7, 7, {
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0,
  });
  const StructuringElement square_5x5(5, 5, std::vector<int>(25, 1));
  const BinaryImage result = dilate(input, square_5x5);

  for (std::size_t row = 0; row < result.rows(); ++row) {
    for (std::size_t col = 0; col < result.cols(); ++col) {
      assert(result.at(row, col) == (row >= 1 && row <= 5 && col >= 1 && col <= 5));
    }
  }

  const StructuringElement cross_3x3(3, 3, {
    0, 1, 0,
    1, 1, 1,
    0, 1, 0,
  });
  const BinaryImage cross_result = dilate(input, cross_3x3);
  assert(cross_result.at(2, 3) == 1 && cross_result.at(3, 2) == 1);
  assert(cross_result.at(2, 2) == 0);

  std::cout << "5x5 dilation result:\n";
  print(result);
}
