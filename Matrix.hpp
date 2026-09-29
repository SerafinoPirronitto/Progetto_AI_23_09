#pragma once

#include <cstddef>
#include <iomanip>
#include <vector>
#include <exception>
#include <concepts>
#include <ostream>
#include <string>
#include <format>
#include <bit>


template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
class Matrix
{
    size_t N;
    size_t M;
    std::vector<T> m_data; // represent data as a flat array (contigous in memory)

public:
    Matrix(size_t rows, size_t cols, T val = T{})
        : N(rows), M(cols), m_data(rows * cols, val) {}


    T& operator[](size_t row, size_t column)
    {
        if(row >= N || column >= M) throw std::invalid_argument("Index out of range");
        return m_data[row*M + column];
    }
    const T& operator[](size_t row, size_t column) const
    {
        if(row >= N || column >= M) throw std::invalid_argument("Index out of range");
        return m_data[row*M + column];
    }


    friend std::ostream& operator<<(std::ostream& out, const Matrix<T>& mat)
    {
        constexpr size_t width = 7;
        out << "\n\n";
        out << "|" << std::setw(width) << " " << " |";
        for (size_t i = 0; i < mat.M; ++i) {
            out << std::right << std::setw(width) << "Col " +  std::to_string(i+1) << " |";
        }
        out << "\n";
        size_t total_width = width + 3 + ((width + 3) * mat.M);
        out << std::setfill('-') << std::setw(total_width) << "-";
        out << std::setfill(' ');
        for (size_t i = 0; i < mat.N*mat.M; i++)
        {
            if (i % mat.M == 0)
            {
               out  << "\n" << "|" << std::left << std::setw(width) << " Row " + std::to_string(i/mat.M+1) << "|";
            }
            out << std::right << std::setw(width) << mat.m_data[i] << " |";
        }
        out << "\n\n";
        return out;
    }
};


template <>
class Matrix<bool>
{
    size_t N;
    size_t M;
    size_t words_per_row; // How many uint64 are needed to represent a single row (1 for M <= 64)
    std::vector<uint64_t> m_data;

    // Proxy class to represent a single bit in memory
    class BitReference {
        uint64_t* word;
        size_t bit_idx;
    public:
        BitReference(uint64_t* w, size_t b) : word(w), bit_idx(b) {}
        
        // Allows writing: matrix[i, j] = true;
        BitReference& operator=(bool value) {
            if (value) {
                *word |= (1ULL << bit_idx);
            } else {
                *word &= ~(1ULL << bit_idx);
            }
            return *this;
        }
        
        // Allows reading: bool x = matrix[r][c];
        operator bool() const {
            return (*word >> bit_idx) & 1ULL;
        }
    };

    public:
        Matrix(size_t rows, size_t cols, bool val = false)
            : N(rows), M(cols), words_per_row((cols+63)/64), m_data(words_per_row*rows, val ? ~0ULL : 0ULL)
        {
            if (val && (M % 64 != 0)) {
                size_t valid_bits_in_last_word = M % 64;
                
                // Create a mask that has 1s only for the valid bits.
                // E.g., if valid_bits is 3, 1ULL << 3 is 8 (1000 in binary).
                // 8 - 1 = 7 (0111 in binary), giving exactly 3 valid bits.
                uint64_t mask = (1ULL << valid_bits_in_last_word) - 1;
                
                // Apply the mask to the last word of every row
                for (size_t r = 0; r < rows; ++r) {
                    size_t last_word_idx = (r * words_per_row) + (words_per_row - 1);
                    m_data[last_word_idx] &= mask;
                }
            }
        }


        BitReference operator[](size_t i, size_t j)
        {
            if (i >= N || j >= M) throw std::invalid_argument("Index out of range");
            size_t word_idx = (i * words_per_row) + (j / 64);
            return BitReference(&m_data[word_idx], j % 64);
        }

        bool operator[](size_t i, size_t j) const
        {
            if (i >= N || j >= M) throw std::invalid_argument("Index out of range");
            size_t word_idx = (i * words_per_row) + (j / 64);
            return (m_data[word_idx] >> (j % 64)) & 1ULL;
        }

        bool is_row_zero(size_t r) const 
        {
            size_t offset = r * words_per_row;
            for (size_t i = 0; i < words_per_row; ++i) 
            {
                if (m_data[offset + i] != 0) return false;
            }
            return true;
        }

        size_t first_one(size_t r) const {
            size_t offset = r * words_per_row;
            for (size_t i = 0; i < words_per_row; ++i) 
            {
                if (m_data[offset + i] != 0) 
                {
                    size_t bit_pos = std::countr_zero(m_data[offset + i]);
                    size_t total_pos = (i * 64) + bit_pos;
                    return total_pos < M ? total_pos : M;
                }
            }
            return M;
        }

        /*
        if Matrix is used to represent an adjacency matrix,
        the (out-)degree of a node n, it's just the number
        of non-zero values in row n.
        */
        size_t count_ones(size_t r) const 
        {
            size_t count = 0;
            size_t offset = r * words_per_row;
            for (size_t i = 0; i < words_per_row; ++i) 
            {
                count += std::popcount(m_data[offset + i]);
            }
            return count;
        }


        friend std::ostream& operator<<(std::ostream& out, const Matrix<bool>& mat)
        {
            constexpr size_t width = 8;
            out << "\n\n";
            out << "|" << std::setw(width) << " " << "|";
            for (size_t i = 0; i < mat.M; ++i) {
                out << std::right << std::setw(width) << " Col " +  std::to_string(i+1) << " |";
            }
            out << "\n";
            size_t total_width = width + 3 + ((width + 3) * mat.M);
            out << std::setfill('-') << std::setw(total_width) << "-";
            out << std::setfill(' ');
            for (size_t i = 0; i < mat.N; i++)
            {
                out  << "\n" << "|" << std::left << std::setw(width) << " Row " + std::to_string(i + 1) << "  |";
                for (size_t j = 0; j < mat.M; j++)
                {
                    out << std::right << std::setw(width) << mat[i, j] << " |";
                }
            }
            out << "\n\n";
            return out;
        }
};