#include <iomanip>
#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <cstring>
#include <cassert>
#include <omp.h>
#include <algorithm>

#define NDEBUG

constexpr const char* fileMatrixA = "matrixA.txt";
constexpr const char* fileMatrixB = "matrixB.txt";
constexpr const char* fileOutput = "output.txt";

using elem = long;
using matrix = std::vector<std::vector<elem>>;

class InputParser{
    public:
        InputParser (int &argc, char **argv){
            for (int i=1; i < argc; ++i)
                this->tokens.push_back(std::string(argv[i]));
        }
        /// @author iain
        const std::string& getCmdOption(const std::string &option) const{
            std::vector<std::string>::const_iterator itr;
            itr =  std::find(this->tokens.begin(), this->tokens.end(), option);
            if (itr != this->tokens.end() && ++itr != this->tokens.end()){
                return *itr;
            }
            static const std::string empty_string("");
            return empty_string;
        }
        /// @author iain
        bool cmdOptionExists(const std::string &option) const{
            return std::find(this->tokens.begin(), this->tokens.end(), option)
                   != this->tokens.end();
        }
    private:
        std::vector <std::string> tokens;
};

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

void printMatrix(const matrix& C, const char* filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << std::strerror(errno);
        exit(errno);
    }

    for (auto r = 0; r < C.size(); ++r) {
        for (auto c = 0; c < C[0].size(); ++c) {
            file << std::setw(8) << C[r][c];
        }
        file << std::endl;
    }
    file << std::endl;

    file.close();
}

int main(int argc, char** argv) {

    InputParser input(argc, argv);
    if(input.cmdOptionExists("-h") || input.cmdOptionExists("--help")){
        std::cout << "Usage:\n";
        std::cout << "\t-p <num> - num of threads (default 1).\n";
        std::cout << "\t-h, --help - help" << std::endl;
        exit(0);
    }
    const std::string &parallel = input.getCmdOption("-p");
    if (!parallel.empty()){
        auto p = std::stoi(parallel);
        if (p < 1) {
            std::cerr << "Error: wrong args.\n";
            exit(-1);
        }
        omp_set_num_threads(std::stoi(parallel));
    } else {
        omp_set_num_threads(1);
    }

    std::cerr << "Loading matrix A\n";
    matrix A = readMatrix(fileMatrixA);
    std::cerr << "Loading matrix B\n";
    matrix B = readMatrix(fileMatrixB);

    // std::cout << "Matrix A:" << std::endl;
    // printMatrix(A);
    // std::cout << "Matrix B:" << std::endl;
    // printMatrix(B);

    const auto widthA = A[0].size();
    const auto heightA = A.size();
    if (widthA != B.size()) {
        std::cerr << "Error: different matrix sizes\n";
        exit(-2);
    }

    const auto widthB = B[0].size();
    const auto heightB = B.size();
    
    matrix C(heightA, std::vector<elem>(widthB, 0));
    // printMatrix(C);

    std::cerr << "Calculating matrix C\n";
#pragma omp parallel shared(C)
{
    #pragma omp for schedule(dynamic)
    for (size_t k = 0; k < heightA * widthB; ++k) {
        auto c = k % widthB;
        auto r = k / heightA;
        // std::cout << "row: " << r << "col: " << c << std::endl;
        assert(r < heightA && c < widthB);
        C[r][c] = compute(Pos(r, c), A, B);
    }
}

    // std::cout << "Matrix C = AxB:" << std::endl;
    std::cerr << "Done.\n";
    printMatrix(C, fileOutput);

    return 0;
    
}