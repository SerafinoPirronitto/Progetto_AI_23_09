#pragma once
#include "Matrix.hpp"


class Solution
{   
    Matrix<bool> m_data;            // ACTUAL SOLUTION
    Matrix<bool> D;                 // DOMAIN
    const Matrix<bool>& m_adj;      // CONSTRAINT MATRIX 
    size_t N, M;


    // PROXY OBJECT FOR CLEAN ASSIGNMENT SYNTAX AND CONSTRAINT PROPAGATION
    struct VariableAssignment
    {
        Solution& owner;
        size_t i;
        size_t j;

        VariableAssignment& operator=(bool v)
        {
            if(v)   owner.assign(i,j);
            else    owner.m_data[i, j] = false;
            return *this;
        }

        operator bool() const {
            return owner.m_data[i, j];
        }
    };

    void assign(size_t event, size_t slot)
    {
        if(m_data[event, slot]) return;

        m_data[event, slot] = true;
        for (size_t k = 0; k < N; ++k)
        {
            if(m_adj[event, k]) 
                D[k, slot] = false;
        }
        assignment_order.emplace_back(event);
    }

public:
    std::vector<size_t> assignment_order;

    Solution(size_t rows, size_t cols, const Matrix<bool>& Adj)
        : m_data(rows, cols, 0), D(rows, cols, 1), m_adj(Adj), N(rows), M(cols) {}

    Solution(const Solution& other)
        : m_data(other.m_data), D(other.D), m_adj(other.m_adj), N(other.N), M(other.M), assignment_order(other.assignment_order) {}

    Solution& operator=(const Solution& other)
    {
        if(this == &other) return *this;
        if(&m_adj != &other.m_adj) 
            throw std::invalid_argument("Cannot copy assing solution to a different problem.");

        N = other.N;
        M = other.M;
        m_data = other.m_data;
        assignment_order = other.assignment_order;
        D = other.D;

        return *this;
    }

    VariableAssignment operator[](size_t i, size_t j)
    {
        return VariableAssignment{*this, i, j};
    }

    bool operator[](size_t i, size_t j) const
    {
        return m_data[i, j];
    }
    
    bool is_complete() const
    {
        for (size_t i = 0; i < N; i++)
        {
            if (m_data.is_row_zero(i)) return false; 
        }
        return true;
    }

    
    const Matrix<bool>& domain() const { return D; }
    const Matrix<bool>& data() const { return m_data; }
};