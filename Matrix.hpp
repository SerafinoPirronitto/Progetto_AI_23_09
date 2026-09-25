#pragma once

#include <cstddef>
#include <array>
#include <exception>
#include <concepts>
#include <ostream>
#include <string>
#include <format>


template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T, std::size_t N, std::size_t M>
class Matrix
{
    std::array<T, N*M> m_data{}; // represent data as a flat array (contigous in memory)

public:
    constexpr const T& operator[](std::size_t row, std::size_t column) const
    {
        if(row >= N || column >= M) throw std::invalid_argument("Index out of range");
        return m_data[row*M + column];
    }
    constexpr T& operator[](std::size_t row, std::size_t column)
    {
        if(row >= N || column >= M) throw std::invalid_argument("Index out of range");
        return m_data[row*M + column];
    }

    friend std::ostream& operator<<(std::ostream& out, const Matrix<T,N,M>& mat) const
    {
        constexpr std::size_t width = 7;
        out << "\n\n";
        out << "|" << std::setw(width) << " " << " |";
        for (size_t i = 0; i < M; ++i) {
            out << std::right << std::setw(width) << "Col " +  std::to_string(i) << " |";
        }
        out << "\n";
        std::size_t total_width = width + 3 + ((width + 3) * M);
        out << std::setfill('-') << std::setw(total_width) << "-";
        out << std::setfill(' ');
        for (std::size_t i = 0; i < N*M; i++)
        {
            if (i % M == 0)
            {
               out  << "\n" << "|" << std::left << std::setw(width) << " Row " + std::to_string(i/N) << "|";
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