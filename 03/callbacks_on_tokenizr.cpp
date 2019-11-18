#include <functional>
#include <cassert>
#include <regex>
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
//      );о.

/**
 * @brief  К сожалению, у нас не все типы чисел, нужно на уровне токенизации
 * разбирать, и допускать только числа из цифр.
 *
 * @param ch
 * @return true
 * @return false
 */

bool is_digit(char ch) {
    int digitmaybe = ch - '0';
    if (0 <= digitmaybe && digitmaybe <= 9) return true;
    return false;
}

/**
 * @brief Проверка символа на вайтспейсность.
 *
 * @param ch
 * @return true — вайтспейс всех видов
 * @return false — что-то осмысленное.
 */
bool is_whitespace(char ch) {
    switch (ch) {
    case ' ':
    case '\t':
    case '\r':
    case '\0':
    case '\n':
        return true;
    }
    return false;
}

class Tokenzr {
  private:
    //Коллбеки. Решил попробовать std::function, никогда раньше не пробовал.
    std::function<void()> onStart_;
    std::function<void()> onFinish_;
    std::function<void(const long &value)> onNumber_;
    std::function<void(const std::string &value)> onString_;

    /**
     * @brief После выделения по вайтспейсам токена, пытаемся понять, число ли
     * он, и вызвать соотвествующие обработчики.
     *
     * @param token
     */
    void process_token(const std::string &token) {
        if (token.empty()) return;
        // да, все можно без регеэспов сделать, но хотел попробовать как с ними
        // в плюсах
        std::regex number_re("(\\d+)");
        if (std::regex_match(token, number_re)) {
            try {
                // Тут конечно по уму, нужно тащить длинную арифметику, но
                // допустим все ложиться в long.
                long number = std::stoi(token);
                if (onNumber_) onNumber_(number);
                return;
            } catch (std::invalid_argument &ex_) {
                // регекспу соответствует но не распарсилось. Ерунда
                // какая-то.
                assert(0);
            } catch (std::out_of_range &ex_) {
                // А вот так точно не договаривались. Число, но не лезет в
                // стандартное.
                assert(0);
            }
        }
        if (onString_) onString_(token);
    }

  public:
    /**
     * @brief Construct a new Tokenzr object
     *
     * @param onStart — коллбек перед парсингом.
     * @param onFinish — коллбек после парсинга
     * @param onNumber — коллбек, если найдено число
     * @param onString — коллбек, если найдена строка
     */

    Tokenzr(){};

    /**
     * @brief Регистрация коллбека для предпарсинга
     *
     * @param onStart
     */
    void register_onStart(std::function<void()> onStart) {
        onStart_ = onStart;
    };

    /**
     * @brief Регистрация коллбека постпарсинга
     *
     * @param onFinish
     */
    void register_onFinish(std::function<void()> onFinish) {
        onFinish_ = onFinish;
    };

    /**
     * @brief Регистрация коллбека на токен-число.
     *
     * @param onNumber
     */
    void register_onNumber(std::function<void(const long &value)> onNumber) {
        onNumber_ = onNumber;
    }

    /**
     * @brief Регистрация коллбека на токен-строку.
     *
     * @param onString
     */
    void
    register_onString(std::function<void(const std::string &value)> onString) {
        onString_ = onString;
    }

    /**
     * @brief Разбор входного текста
     *
     * @param text
     */
    void parse(const std::string &text) {
        // http://www.cplusplus.com/reference/functional/function/operator_bool/

        if (onStart_) onStart_();

        std::string token("");
        enum STATE { WHITESPACE, MAYBE_NUMBER, STRING };
        STATE state = WHITESPACE;

        for (auto ch = text.begin(); ch <= text.end(); ch++) {
            if (WHITESPACE == state) {
                if (is_whitespace(*ch)) continue;
                if (is_digit(*ch))
                    state = MAYBE_NUMBER;
                else
                    state = STRING;
                token = *ch;
            } else if (MAYBE_NUMBER == state) {
                if (is_digit(*ch)) {
                    token += *ch;
                    continue;
                } else if (is_whitespace(*ch)) {
                    process_token(token);
                    state = WHITESPACE;
                    token = "";
                    continue;
                } else {
                    state = STRING;
                    token += *ch;
                    continue;
                }
            } else {
                // Тут строки
                if (is_whitespace(*ch)) {
                    process_token(token);
                    state = WHITESPACE;
                    token = "";
                    continue;
                } else {
                    token += *ch;
                    continue;
                }
            }
            process_token(token);
        }
        if (onFinish_) onFinish_();
    }
};
std::stringstream testout("");

void testStart_() {
    testout << "***** BEGIN *****" << std::endl << std::flush;
};

void testFinish_() {
    testout << "***** END *****" << std::endl << std::flush;
};

void testNumber_(const long &value) {
    testout << "#" << value << "#" << std::endl << std::flush;
};

void testString_(const std::string &value) {
    testout << "<" << value << ">" << std::endl << std::flush;
};

TEST_CASE("Горячий тест — что сломалось в последний раз", "[tok]") {
    Tokenzr tok;
    const char *wtf_text = R"0x1.tv(
    )0x1.tv";

    SECTION("Проверяем только строки") {
        testout.str("");
        tok.register_onString(testString_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv()0x1.tv" == testout.str());
    }
}

TEST_CASE("Тестируем токенайзер постепенно добавляя коллбеки", "[tok]") {
    Tokenzr tok;
    const char *wtf_text = R"0x1.tv(
  strinh34s 1234  -3445 56474 dsfasfgbbljkdfglkdf

  fgdfgd;
  fggdfg;;  +56547 
  -45666
    )0x1.tv";

    testout.str("");
    tok.parse(wtf_text);
    REQUIRE("" == testout.str());

    SECTION("Проверяем старт коллбек") {
        tok.register_onStart(testStart_);
        tok.parse(wtf_text);
        //         std::string wtf(testout.str());
        //         std::string wtf2(R"0x1.tv(***** BEGIN *****
        // )0x1.tv");
        REQUIRE(R"0x1.tv(***** BEGIN *****
)0x1.tv" == testout.str());
        // std::cout << testout.str() << std::endl;
    }

    SECTION("Проверяем финиш коллбек") {
        testout.str("");
        tok.register_onFinish(testFinish_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv(***** END *****
)0x1.tv" == testout.str());
    }

    SECTION("Проверяем старт-финиш") {
        testout.str("");
        tok.register_onStart(testStart_);
        tok.register_onFinish(testFinish_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv(***** BEGIN *****
***** END *****
)0x1.tv" == testout.str());
    }

    SECTION("Проверяем только числа") {
        testout.str("");
        tok.register_onNumber(testNumber_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv(#1#
#1234#
#5#
#56474#
)0x1.tv" == testout.str());
    }

    SECTION("Проверяем только строки") {
        testout.str("");
        tok.register_onString(testString_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv(<s>
<strinh34s>
<->
<-3445>
<d>
<dsfasfgbbljkdfglkdf>
<f>
<fgdfgd;>
<f>
<fggdfg;;>
<+>
<+56547>
<->
<-45666>
)0x1.tv" == testout.str());
    }

    SECTION("Проверяем строки и числа") {
        testout.str("");
        tok.register_onString(testString_);
        tok.register_onNumber(testNumber_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv(<s>
<strinh34s>
#1#
#1234#
<->
<-3445>
#5#
#56474#
<d>
<dsfasfgbbljkdfglkdf>
<f>
<fgdfgd;>
<f>
<fggdfg;;>
<+>
<+56547>
<->
<-45666>
)0x1.tv" == testout.str());
    }

    SECTION("Проверяем все коллбеки") {
        testout.str("");
        tok.register_onString(testString_);
        tok.register_onNumber(testNumber_);
        tok.register_onStart(testStart_);
        tok.register_onFinish(testFinish_);
        tok.parse(wtf_text);
        REQUIRE(R"0x1.tv(***** BEGIN *****
<s>
<strinh34s>
#1#
#1234#
<->
<-3445>
#5#
#56474#
<d>
<dsfasfgbbljkdfglkdf>
<f>
<fgdfgd;>
<f>
<fggdfg;;>
<+>
<+56547>
<->
<-45666>
***** END *****
)0x1.tv" == testout.str());
    }
}
