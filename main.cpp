#include <cassert>
#include <cstddef>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
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

using Offset = std::pair<std::ptrdiff_t, std::ptrdiff_t>;

class StructuringElement {
public:
  StructuringElement(std::size_t rows, std::size_t cols, const std::vector<int> & mask)
  {
    if (rows == 0 || cols == 0 || rows % 2 == 0 || cols % 2 == 0 || mask.size() != rows * cols) {
      throw std::invalid_argument("structuring element must have odd dimensions");
    }
    for (std::size_t index = 0; index < mask.size(); ++index) {
      const int value = mask[index];
      if (value != 0 && value != 1) {
        throw std::invalid_argument("structuring element values must be 0 or 1");
      }
      if (value == 1) {
        active_offsets_.emplace_back(
          static_cast<std::ptrdiff_t>(index / cols) - static_cast<std::ptrdiff_t>(rows / 2),
          static_cast<std::ptrdiff_t>(index % cols) - static_cast<std::ptrdiff_t>(cols / 2));
      }
    }
  }

  const std::vector<Offset> & offsets() const { return active_offsets_; }

private:
  std::vector<Offset> active_offsets_;
};

bool isForeground(const BinaryImage & image, std::ptrdiff_t row, std::ptrdiff_t col)
{
  return row >= 0 && col >= 0 &&
         row < static_cast<std::ptrdiff_t>(image.rows()) &&
         col < static_cast<std::ptrdiff_t>(image.cols()) &&
         image.at(static_cast<std::size_t>(row), static_cast<std::size_t>(col)) == 1;
}

BinaryImage dilate(const BinaryImage & image, const StructuringElement & element)
{
  std::vector<int> output(image.rows() * image.cols(), 0);
  for (std::size_t index = 0; index < output.size(); ++index) {
    const auto row = static_cast<std::ptrdiff_t>(index / image.cols());
    const auto col = static_cast<std::ptrdiff_t>(index % image.cols());
    for (const auto & [row_offset, col_offset] : element.offsets()) {
      if (isForeground(image, row + row_offset, col + col_offset)) {
        output[index] = 1;
        break;
      }
    }
  }
  return BinaryImage(image.rows(), image.cols(), std::move(output));
}

BinaryImage erode(const BinaryImage & image, const StructuringElement & element)
{
  std::vector<int> output(image.rows() * image.cols(), 1);
  for (std::size_t index = 0; index < output.size(); ++index) {
    const auto row = static_cast<std::ptrdiff_t>(index / image.cols());
    const auto col = static_cast<std::ptrdiff_t>(index % image.cols());
    for (const auto & [row_offset, col_offset] : element.offsets()) {
      if (!isForeground(image, row + row_offset, col + col_offset)) {
        output[index] = 0;
        break;
      }
    }
  }
  return BinaryImage(image.rows(), image.cols(), std::move(output));
}

BinaryImage readImage(std::istream & input)
{
  long long entered_rows;
  long long entered_cols;
  std::cout << "请输入图像尺寸（行 列）：";
  if (!(input >> entered_rows >> entered_cols) || entered_rows <= 0 || entered_cols <= 0) {
    throw std::invalid_argument("expected positive row and column counts");
  }

  const auto rows = static_cast<std::size_t>(entered_rows);
  const auto cols = static_cast<std::size_t>(entered_cols);
  if (cols > std::numeric_limits<std::size_t>::max() / rows) {
    throw std::invalid_argument("image dimensions are too large");
  }

  std::vector<int> pixels(rows * cols);
  std::cout << "请输入 " << pixels.size() << " 个 0/1 像素：\n";
  for (int & pixel : pixels) {
    if (!(input >> pixel)) {
      throw std::invalid_argument("not enough pixel values");
    }
  }
  return BinaryImage(rows, cols, std::move(pixels));
}

void print(const BinaryImage & image)
{
  for (std::size_t row = 0; row < image.rows(); ++row) {
    for (std::size_t col = 0; col < image.cols(); ++col) {
      std::cout << image.at(row, col) << (col + 1 == image.cols() ? '\n' : ' ');
    }
  }
}

void runSelfCheck()
{
  const BinaryImage single_pixel(7, 7, {
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
  });
  const StructuringElement square_5x5(5, 5, std::vector<int>(25, 1));
  const BinaryImage result = dilate(single_pixel, square_5x5);

  for (std::size_t row = 0; row < result.rows(); ++row) {
    for (std::size_t col = 0; col < result.cols(); ++col) {
      assert(result.at(row, col) == (row >= 1 && row <= 5 && col >= 1 && col <= 5));
    }
  }
  const BinaryImage recovered = erode(result, square_5x5);
  for (std::size_t row = 0; row < recovered.rows(); ++row) {
    for (std::size_t col = 0; col < recovered.cols(); ++col) {
      assert(recovered.at(row, col) == single_pixel.at(row, col));
    }
  }

  const StructuringElement cross_3x3(3, 3, {
    0, 1, 0,
    1, 1, 1,
    0, 1, 0,
  });
  const BinaryImage cross_result = dilate(single_pixel, cross_3x3);
  assert(cross_result.at(2, 3) == 1 && cross_result.at(3, 2) == 1);
  assert(cross_result.at(2, 2) == 0);

  const StructuringElement disk_5x5(5, 5, {
    0, 0, 1, 0, 0,
    0, 1, 1, 1, 0,
    1, 1, 1, 1, 1,
    0, 1, 1, 1, 0,
    0, 0, 1, 0, 0,
  });
  const BinaryImage rounded = dilate(single_pixel, disk_5x5);
  for (std::size_t row = 0; row < rounded.rows(); ++row) {
    for (std::size_t col = 0; col < rounded.cols(); ++col) {
      const int row_offset = static_cast<int>(row) - 3;
      const int col_offset = static_cast<int>(col) - 3;
      assert(rounded.at(row, col) == (row_offset * row_offset + col_offset * col_offset <= 4));
    }
  }
}

BinaryImage demoImage()
{
  return BinaryImage(7, 7, {
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0,
  });
}

int main(int argc, char * argv[])
{
  try {
    runSelfCheck();
    bool use_demo = false;
    bool use_erosion = false;
    bool use_rounded = false;
    for (int index = 1; index < argc; ++index) {
      const std::string option(argv[index]);
      if (option == "--demo") {
        use_demo = true;
      } else if (option == "--erode") {
        use_erosion = true;
      } else if (option == "--rounded") {
        use_rounded = true;
      } else {
        throw std::invalid_argument("supported options are --demo, --erode, and --rounded");
      }
    }

    const BinaryImage input = use_demo ? demoImage() : readImage(std::cin);
    const StructuringElement square_5x5(5, 5, std::vector<int>(25, 1));
    const StructuringElement disk_5x5(5, 5, {
      0, 0, 1, 0, 0,
      0, 1, 1, 1, 0,
      1, 1, 1, 1, 1,
      0, 1, 1, 1, 0,
      0, 0, 1, 0, 0,
    });
    const StructuringElement & element = use_rounded ? disk_5x5 : square_5x5;
    const std::string operation_name = use_erosion
      ? (use_rounded ? "圆角结构元素腐蚀" : "正方形结构元素腐蚀")
      : (use_rounded ? "圆角膨胀" : "正方形膨胀");
    std::cout << "\n----- " << operation_name << "结果 -----\n";
    print(use_erosion ? erode(input, element) : dilate(input, element));
  } catch (const std::exception & error) {
    std::cerr << "Usage: [--demo] [--erode] [--rounded] with rows cols and binary pixels on standard input\n";
    std::cerr << "Error: " << error.what() << '\n';
    return 1;
  }
}
