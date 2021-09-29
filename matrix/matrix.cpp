#include <iomanip>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cassert>

#define NDEBUG

constexpr const char* fileMatrixA = "matrixA.txt";
constexpr const char* fileMatrixB = "matrixB.txt";

using elem = int;
using matrix = std::vector<std::vector<elem>>;

class Pos {
public:
    Pos(const size_t row, const size_t col) {
        this->row = row;
        this->col = col;
    }

    size_t row{0};
    size_t col{0};
};

std::vector<elem> readRow(std::string row) {
  std::vector<elem> retval;
  std::istringstream is(row);
  elem num;
  while (is >> num) {
      // std::cout << num << std::endl;
      retval.push_back(num);
  }
  return retval;
}

/*
std::vector<std::vector<int>> readVector(std::istream &is) {
  std::string line;
  std::vector<std::vector<int>> retval;
  while (std::getline(is, line))
    retval.push_back(readRow(line));
  return retval;
}
*/

matrix readMatrix(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << std::strerror(errno);
        exit(errno);
    }

    matrix retarr;
    std::string line;
    auto arr_size = 0;
    while (std::getline(file, line)) {
        const auto row = readRow(line);
        if (arr_size == 0) {
            arr_size = row.size();
        } else if (arr_size != row.size()) {
            std::cerr << "Error: wrong matrix size\n";
            exit(-1);
        }
        retarr.push_back(row);
    }
    file.close();
    if (arr_size == 0) {
        std::cerr << "Error: matrix is empty\n";
        exit(-3);
    }

    /*
    if (arr_size > 0 && arr_size != retarr.size()) {
        std::cerr << "wrong matrix size " << retarr.size() << std::endl;
        exit(-1);
    }
    */

    return retarr;
}

elem compute(const Pos& idx, const matrix& A, const matrix& B) {
    elem result = 0;
    for (auto i = 0; i < B.size(); ++i) {
        //std::cerr << i << " " << idx.row << " " << idx.col << std::endl; 
        result += A[idx.row][i] * B[i][idx.col];
    }
    return result;
}

void printMatrix(const matrix& C) {
    // empty processed correctly
    for (auto r = 0; r < C.size(); ++r) {
        for (auto c = 0; c < C[0].size(); ++c) {
            std::cout << std::setw(5) << C[r][c];
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

int main() {

    matrix A = readMatrix(fileMatrixA);
    matrix B = readMatrix(fileMatrixB);

    printMatrix(A);
    printMatrix(B);
    const auto widthA = A[0].size();
    const auto heightA = A.size();
    if (widthA != B.size()) {
        std::cerr << "Error: different matrix sizes\n";
        exit(-2);
    }

    matrix C(heightA);
    const auto widthB = B[0].size();
    const auto heightB = B.size();
    for (size_t k = 0; k < heightA * widthB; ++k) {
        auto c = k % widthB;
        auto r = k / heightA;
        //std::cout << "row: " << r << "col: " << c << std::endl;
        assert(r < heightA && c < widthB);
        C[r].push_back(compute(Pos(r, c), A, B));
    }

    printMatrix(C);

    return 0;
    
}