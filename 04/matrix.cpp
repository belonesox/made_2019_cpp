#include <cstring>
#include <iostream>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

/**
 * @brief Основной класс реализации плотной матрицы, с построчным хранением
 *
 */
class Matrix {
  private:
    int *m_buffer;
    uint m_rowsnum;
    uint m_colsnum;

    /**
     * @brief Прокси-класс для реализации строки-матрицы, чтобы реализовать
     * цепочку операторов []
     *
     */
    class MatrixRow {
      private:
        int *m_buffer;
        uint m_colsnum;

      public:
        MatrixRow(int *buffer, uint colsnum)
            : m_colsnum(colsnum), m_buffer(buffer){};

        int operator[](const uint col) const {
            if (col >= m_colsnum) {
                throw std::out_of_range("");
            }

            return m_buffer[col];
        }

        int &operator[](const uint col) {
            if (col >= m_colsnum) {
                throw std::out_of_range("");
            }

            return m_buffer[col];
        }
    };

  public:
    Matrix(uint rowsnum, uint colsnum)
        : m_rowsnum(rowsnum), m_colsnum(colsnum) {

        m_buffer = new int[m_rowsnum * m_colsnum]();
    }

    ~Matrix() {
        delete[] m_buffer;
    };

    int get_rows_num() const {
        return m_rowsnum;
    };
    int get_columns_num() const {
        return m_colsnum;
    };

    /**
     * @brief Оператор равенства
     *
     * @param matrix
     * @return true
     * @return false
     */
    bool operator==(const Matrix &matrix) const {
        if (this == &matrix) return true;

        if (m_rowsnum != matrix.m_rowsnum || m_colsnum != matrix.m_colsnum)
            return false;

        return std::memcmp(m_buffer, m_buffer,
                           m_colsnum * m_rowsnum * sizeof(int)) == 0;
    }

    /**
     * @brief Оператор неравенства, определим как обратный к равенству.
     *
     * @param matrix
     * @return true
     * @return false
     */
    bool operator!=(const Matrix &matrix) const {
        return !(*this == matrix);
    };

    /**
     * @brief Присваивание скаляра матрице
     *
     * @param value
     * @return Matrix&
     */
    Matrix &operator*=(const int value) {
        for (uint i = 0; i < m_colsnum * m_rowsnum; i++) {
            m_buffer[i] *= value;
        }
        return *this;
    }

    /**
     * @brief Выбор строки по номеру для операций в правой части
     *
     * @param rownum
     * @return const MatrixRow
     */
    const MatrixRow operator[](const uint rownum) const {
        if (rownum >= m_rowsnum) throw std::out_of_range("");

        const MatrixRow row(&m_buffer[rownum * m_colsnum], m_colsnum);
        return row;
    }

    /**
     * @brief Выбор строки по номеру для модификаций
     *
     *
     * @param rownum
     * @return MatrixRow
     */
    MatrixRow operator[](const uint rownum) {
        if (rownum >= m_rowsnum) throw std::out_of_range("");

        MatrixRow row(&m_buffer[rownum * m_colsnum], m_colsnum);
        return row;
    }
};

Matrix &load_matrix_from_string(std::string &str) {
}

TEST_CASE("Горячий тест — что сломалось в последний раз", "[matrix]") {
    Matrix mat(3, 5);
    const char *mat1_str = R"0x1.tv(
1 0 0 0 4
0 1 0 0 3
0 2 3 0 5 
    )0x1.tv";

    const char *mat2_str = R"0x1.tv(
1 0 0 0 
0 1 0 0 
0 2 3 0  
    )0x1.tv";

    REQUIRE(1);

    //     SECTION("Проверяем только строки") {
    //         testout.str("");
    //         tok.register_onString(testString_);
    //         tok.parse(wtf_text);
    //         REQUIRE(R"0x1.tv()0x1.tv" == testout.str());
    //     }
}
