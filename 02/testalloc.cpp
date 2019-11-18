
#include <iostream>
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

// https://google.github.io/styleguide/cppguide.html
// Спросить:
// *  Есть styleguide с префиксами/постфиксами для class members? (помню, что
// без них было неудобно, а разные варианты приводили к спорам)
// *  Есть ли более-менее стандартные утилиты генерации заголовочных файлов из
// CPP?
//      DRY хотелось бы соблюдать, но lzz и preprocess и что-то еще не
//      впечатлило. Завязываться на свойства IDE («сгенерировать класс»), тоже
//      неправильно.

/**
 * @brief Аллокатор со стратегией линейного выделения памяти
 *
 */
class LinearAllocator {
  private:
    size_t m_max_size;
    char *m_buffer;
    char *m_position;

  public:
    /**
     * @brief Construct a new Linear Allocator object
     *
     * @param maxSize
     *
     *  Если размер нулевой (отрицательного быть не может),
     *  не будем брать SIZE_MAX (как в malloc) захватывая всю память, делаем
     * вырожденный аллокатор, не возвращающий ничего.
     *
     */
    LinearAllocator(size_t maxSize)
        : m_max_size(0), m_position(nullptr), m_buffer(nullptr) {
        if (maxSize > 0) m_max_size = maxSize;
        m_position = m_buffer = (char *)malloc(m_max_size);
    }

    /**
     * @brief   Выделение заданной памяти.
     *          В отличие от malloc отрицательные и нулевые размеры не
     * поддерживаем, если что не ОК, возвращаем nullptr.
     *
     * @param size
     * @return char*
     */
    char *alloc(size_t size) {
        if (nullptr == m_buffer) return nullptr;
        if (size == 0) return nullptr;
        // Вроде так переполнения не поймать, поправьте меня если ---
        size_t rest_size = m_max_size - (m_position - m_buffer);
        if (size > rest_size) return nullptr;

        char *result = m_position;
        m_position += size;
        return result;
    }

    /**
     * @brief Быстрое освобождение всей выделенной пользователям памяти.
     *
     */
    void reset() {
        m_position = m_buffer;
    }

    ~LinearAllocator() {
        if (m_buffer != nullptr) free(m_buffer);
    };
};

TEST_CASE("LinearAllocator с нулевым размером должен всегда возвращать NULL",
          "[la]") {
    LinearAllocator la(0);
    REQUIRE(nullptr == la.alloc(1));
    REQUIRE(nullptr == la.alloc(-1));
    SECTION("reset even cannot revive bad allocator") {
        la.reset();
        REQUIRE(nullptr == la.alloc(1));
        REQUIRE(nullptr == la.alloc(-1));
    }
}

TEST_CASE("LinearAllocator с нормальным размером", "[la]") {
    LinearAllocator la(127);
    REQUIRE(nullptr != la.alloc(1));
    REQUIRE(nullptr != la.alloc(2));
    REQUIRE(nullptr != la.alloc(4));
    REQUIRE(nullptr != la.alloc(8));
    REQUIRE(nullptr != la.alloc(16));
    REQUIRE(nullptr != la.alloc(32));
    REQUIRE(nullptr != la.alloc(64));
    REQUIRE(nullptr == la.alloc(1));
    SECTION("reset должен его оживить") {
        la.reset();
        REQUIRE(nullptr != la.alloc(8));
        REQUIRE(nullptr != la.alloc(64));
        REQUIRE(nullptr != la.alloc(4));
        REQUIRE(nullptr != la.alloc(32));
        REQUIRE(nullptr != la.alloc(2));
        REQUIRE(nullptr != la.alloc(16));
        REQUIRE(nullptr != la.alloc(1));
        REQUIRE(nullptr == la.alloc(1));
    }
    SECTION("Не должно ничего утекать/ломаться при регулярных reset") {
        for (auto i = 0; i < 100000; i++) {
            la.reset();
            REQUIRE(nullptr != la.alloc(8));
            REQUIRE(nullptr != la.alloc(64));
            REQUIRE(nullptr != la.alloc(4));
            REQUIRE(nullptr != la.alloc(32));
            REQUIRE(nullptr != la.alloc(2));
            REQUIRE(nullptr != la.alloc(16));
            REQUIRE(nullptr != la.alloc(1));
            REQUIRE(nullptr == la.alloc(1));
        }
    }
}

TEST_CASE(
    "LinearAllocator with incorrect negative size should always return NULL",
    "[la]") {
    LinearAllocator la(-17);
    REQUIRE(nullptr == la.alloc(1));
    REQUIRE(nullptr == la.alloc(-1));
    SECTION("reset even cannot revive bad allocator") {
        la.reset();
        REQUIRE(nullptr == la.alloc(1));
        REQUIRE(nullptr == la.alloc(-1));
    }
}
