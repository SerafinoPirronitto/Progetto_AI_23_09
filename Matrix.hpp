#pragma once

#include <cstddef>
#include <iomanip>
#include <vector>
#include <exception>
#include <concepts>
#include <ostream>
#include <string>
#include <format>


template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
class Matrix
{
    std::size_t N;
    std::size_t M;
    std::vector<T> m_data; // represent data as a flat array (contigous in memory)

public:
    Matrix(std::size_t rows, std::size_t cols, T val = T{})
        : N(rows), M(cols), m_data(rows * cols, val) {}

    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;

    constexpr reference operator[](std::size_t row, std::size_t column)
    {
        if(row >= N || column >= M) throw std::invalid_argument("Index out of range");
        return m_data[row*M + column];
    }
    constexpr const_reference operator[](std::size_t row, std::size_t column) const
    {
        if(row >= N || column >= M) throw std::invalid_argument("Index out of range");
        return m_data[row*M + column];
    }


    friend std::ostream& operator<<(std::ostream& out, const Matrix<T>& mat)
    {
        constexpr std::size_t width = 7;
        out << "\n\n";
        out << "|" << std::setw(width) << " " << " |";
        for (size_t i = 0; i < mat.M; ++i) {
            out << std::right << std::setw(width) << "Col " +  std::to_string(i+1) << " |";
        }
        out << "\n";
        std::size_t total_width = width + 3 + ((width + 3) * mat.M);
        out << std::setfill('-') << std::setw(total_width) << "-";
        out << std::setfill(' ');
        for (std::size_t i = 0; i < mat.N*mat.M; i++)
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


    /*
    if Matrix is used to represent an adjacency matrix,
    the (out-)degree of a node n, it's just the number
    of non-zero values in row n.
    */
    std::size_t degree(std::size_t n) const
    {
        if(n >= N) throw std::invalid_argument("Index out of range");

        std::size_t count = 0;
        for (std::size_t j = 0; j < M; j++)
            if (m_data[n*M+j] != 0) ++count;
        return count;   
    }

};