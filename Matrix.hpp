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
        
        // Allows reading: bool x = matrix[i, j];
        operator bool() const {
            return (*word >> bit_idx) & 1ULL;
        }
    };

    public:
        Matrix(size_t rows, size_t cols, bool val = false)
            : N(rows), M(cols), m_data((rows * cols + 63) / 64, val ? ~0ULL : 0ULL)
        {
            // Mask the unused bits in the very last word of the entire matrix
            size_t total_bits = N * M;
            if (val && (total_bits % 64 != 0)) 
            {
                size_t valid_bits = total_bits % 64;
                uint64_t mask = (1ULL << valid_bits) - 1;
                m_data.back() &= mask;
            }
        }


        BitReference operator[](size_t i, size_t j)
        {
            if (i >= N || j >= M) throw std::invalid_argument("Index out of range");
            size_t idx = i * M + j;
            return BitReference(&m_data[idx/64], idx % 64);
        }

        bool operator[](size_t i, size_t j) const
        {
            if (i >= N || j >= M) throw std::invalid_argument("Index out of range");
            size_t idx = i * M + j;
            return (m_data[idx/64] >> (idx % 64)) & 1ULL;
        }

        // Helper to process a row efficiently across 64-bit word boundaries
        template<typename Func>
        void process_row_chunks(size_t r, Func func) const 
        {
            size_t current = r * M;
            size_t end = current + M;
            
            while (current < end) {
                size_t word_idx = current / 64;
                size_t bit_offset = current % 64;
                size_t chunk_size = std::min(end - current, 64 - bit_offset);
                
                uint64_t mask = (chunk_size == 64) ? ~0ULL : ((1ULL << chunk_size) - 1);
                uint64_t chunk = (m_data[word_idx] >> bit_offset) & mask;
                
                if (!func(chunk, chunk_size)) break; // stop early if func returns false
                
                current += chunk_size;
            }
        }

        bool is_row_zero(size_t r) const 
        {
            bool is_zero = true;
            process_row_chunks(r, [&](uint64_t chunk, size_t) {
                if (chunk != 0) {
                    is_zero = false;
                    return false; // stop processing
                }
                return true; // continue
            });
            return is_zero;
        }

        size_t first_one(size_t r) const 
        {
            size_t col_offset = 0;
            size_t result = M;
            process_row_chunks(r, [&](uint64_t chunk, size_t chunk_size) {
                if (chunk != 0) {
                    result = col_offset + std::countr_zero(chunk);
                    return false; // stop processing
                }
                col_offset += chunk_size;
                return true; // continue
            });
            return result;
        }

        size_t count_ones(size_t r) const 
        {
            size_t count = 0;
            process_row_chunks(r, [&](uint64_t chunk, size_t) {
                count += std::popcount(chunk);
                return true; // continue
            });
            return count;
        }

        friend std::ostream& operator<<(std::ostream& out, const Matrix<bool>& mat)
        {
            constexpr size_t width = 8;
            out << "\n\n|" << std::setw(width) << " " << "|";
            for (size_t i = 0; i < mat.M; ++i) {
                out << std::right << std::setw(width) << " Col " + std::to_string(i+1) << " |";
            }
            out << "\n";
            size_t total_width = width + 3 + ((width + 3) * mat.M);
            out << std::setfill('-') << std::setw(total_width) << "-";
            out << std::setfill(' ');
            for (size_t i = 0; i < mat.N; i++)
            {
                out  << "\n|" << std::left << std::setw(width) << " Row " + std::to_string(i + 1) << "  |";
                for (size_t j = 0; j < mat.M; j++)
                {
                    out << std::right << std::setw(width) << mat[i, j] << " |";
                }
            }
            out << "\n\n";
            return out;
        }
};

