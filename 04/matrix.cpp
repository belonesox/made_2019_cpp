#include <cstring>
#include <iostream>
// Используются только для тестов, boost можно считать стандартной библитекой,
// catch2 принес с собой.
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

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
        m_buffer = nullptr;
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

        auto res = std::memcmp(m_buffer, matrix.m_buffer,
                               m_colsnum * m_rowsnum * sizeof(int));
        return res == 0;
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

/**
 * @brief Для тестов удобней использовать текстовые представления строк,
 *  но чтобы не замусоривать требуемый класс еще одним конструктором, отдельная
 * прокси функция. Плюс целевой класс не тянет зависимостей от boost, который
 * тут хотелось попробовать.
 *
 * @param str
 * @return Matrix*
 */
Matrix *load_matrix_from_string(std::string str) {
    boost::char_separator<char> rowsep("\n");
    boost::char_separator<char> colsep(" ");
    boost::tokenizer<boost::char_separator<char>> lines(str, rowsep);
    uint rows_num = 0;
    uint cols_num = 0;
    BOOST_FOREACH (std::string line, lines) {
        uint cols_num_ = 0;
        auto line_ok_ = false;
        boost::tokenizer<boost::char_separator<char>> cols(line, colsep);
        BOOST_FOREACH (std::string col, cols) {
            line_ok_ = true;
            cols_num_++;
        }
        if (line_ok_) rows_num++;
        cols_num = std::max(cols_num, cols_num_);
    }

    Matrix *pmat = new Matrix(rows_num, cols_num);

    uint i = 0;
    BOOST_FOREACH (std::string line, lines) {
        uint j = 0;
        boost::tokenizer<boost::char_separator<char>> cols(line, colsep);
        BOOST_FOREACH (std::string col, cols) {
            int val = boost::lexical_cast<int>(col);
            (*pmat)[i][j++] = val;
        }
        i++;
    }

    return pmat;
}

TEST_CASE("Горячий тест — что сломалось в последний раз", "[matrix]") {
    Matrix mat(3, 5);
    const char *mat3x5_str = R"0x1.tv(
1 0 0 0 4
0 1 0 0 3
0 2 3 0 5 
    )0x1.tv";

    Matrix mat3x5(*load_matrix_from_string(mat3x5_str));
    Matrix mat3x5_01(*load_matrix_from_string(mat3x5_str));
    REQUIRE(mat3x5 == mat3x5_01);
    mat3x5_01 *= 7;
    auto wtf = mat3x5_01[0][0];
    auto wtf1 = mat3x5_01[2][1];
}

TEST_CASE("Тесты на равенство и умножение", "[matrix]") {
    Matrix mat(3, 5);
    const char *mat3x5_str = R"0x1.tv(
1 0 0 0 4
0 1 0 0 3
0 2 3 0 5 
    )0x1.tv";

    Matrix mat3x5(*load_matrix_from_string(mat3x5_str));
    Matrix mat3x5_same(*load_matrix_from_string(mat3x5_str));

    const char *mat3x4_str = R"0x1.tv(
    1 0 0 0
    0 1 0 0
    0 2 3 0
        )0x1.tv";
    Matrix mat3x4(*load_matrix_from_string(mat3x4_str));

    REQUIRE(5 == mat3x5.get_columns_num());
    REQUIRE(3 == mat3x5.get_rows_num());

    REQUIRE(mat3x5 == mat3x5);
    REQUIRE(mat3x5 == mat3x5_same);
    mat3x5_same[2][3] = 1;
    REQUIRE(mat3x5 != mat3x5_same);
    REQUIRE(mat3x4 != mat3x5);

    Matrix mat3x5_01(*load_matrix_from_string(mat3x5_str));
    REQUIRE(mat3x5 == mat3x5_01);
    mat3x5_01 *= 7;
    REQUIRE(7 == mat3x5_01[0][0]);
    REQUIRE(14 == mat3x5_01[2][1]);
    REQUIRE(21 == mat3x5_01[2][2]);

    REQUIRE(mat3x5 != mat3x5_01);

    mat3x5 *= 0;
    mat3x5_01 *= 0;
    REQUIRE(mat3x5 == mat3x5_01);
    REQUIRE(1);
}

TEST_CASE("Тесты умножение", "[matrix]") {
    Matrix mat(3, 5);
    const char *mat3x5_str = R"0x1.tv(
1 -1 2 -2 3
0 8 13 -45 1
-3 -4 -5 -6 0
    )0x1.tv";

    const char *mat3x5_times_minus7_str = R"0x1.tv(
-7 7 -14 14 -21
0 -56 -91 315 -7
21 28 35 42 0
    )0x1.tv";

    Matrix mat3x5(*load_matrix_from_string(mat3x5_str));
    Matrix mat3x5_times_minus7(
        *load_matrix_from_string(mat3x5_times_minus7_str));

    REQUIRE(mat3x5 != mat3x5_times_minus7);
    mat3x5 *= -7;

    REQUIRE(mat3x5 == mat3x5_times_minus7);

    mat3x5 *= -2;
    REQUIRE(mat3x5 != mat3x5_times_minus7);

    mat3x5 *= -0;
    mat3x5_times_minus7 *= 0;
    REQUIRE(mat3x5 == mat3x5_times_minus7);

    REQUIRE(1);
}
