#include <cassert>
#include <complex>
#include <iomanip>
#include <iostream>
#include <string.h>
#include <unordered_map>
#include <vector>
using namespace std;
#include "bigint.h"

// sudo dnf install -y cppcheck
// sudo dnf install -y clang
// vscode Clang-Format

/*
Используя метод рекурсивного спуска, написать калькулятор. Поддерживаемые
операции:

    умножение
    деление
    сложение
    вычитание
    унарный минус

Для вычислений использовать тип int, приоритет операций стандартный. Передача
выражения осуществляется через аргумент командной строки, поступаемые числа
целые, результат выводится в cout. Пример:

calc "2 + 3 * 4 - -2"

----

Методологическая проблемы.

Если использовать тип int, long int, и даже long long int, будут ошибки
переполнения, а их специально не заказывали и вообще вопрос лимитов не
поднимался — т.е. возможно ожидается прохождение тестов для чисел неограниченной
длины. Я использовал неограниченные инты — BigInt.

Еще открытый вопрос о делении нацело отрицательных чисел — иногда математики
требуют там именно «floor integer division», как в python, но думаю, тут ожидают
простое деление нацелов, в духе -5/2=-2, а не -3.
(Хотя при этом придется дико помучаться, проверяя калькулятор питоном.)
*/

class CSyntaxError {};
class CDivisionByZero {};

class CCalculator {

  public:
    // Основной интерфейс.
    BigInt process(const char *input_expression) {
        if (!input_expression) {
            throw(CSyntaxError());
        }
        number = 0;
        token_type = UNDEF;
        expression = input_expression;
        pos = 0;

        return process_low_precendence();
    }

    // Публичный getter, чтобы знать, где произошла ошибка разбора
    int get_pos() {
        return pos;
    };

  private:
    enum TOKENTYPE {
        UNDEF = 0,
        EOL = '\0',
        NUMBER = 1,
        ADD = '+',
        SUB = '-',
        MUL = '*',
        DIV = '/'
    };

    // в процессе парсинга значения числовых литералов
    BigInt number;

    // Текущая позиция в разборе
    int pos;

    const char *expression;
    TOKENTYPE token_type;

    BigInt process_low_precendence() {
        BigInt res = process_high_precendence();

        while (1) {
            switch (next_token()) {
            case '+':
                res += process_high_precendence();
                break;
            case '-':
                res -= process_high_precendence();
                break;
            case EOL:
                return res;
            default:
                throw(CSyntaxError());
            }
        }
        return 0;
    }

    BigInt process_high_precendence() {
        BigInt res = process_number();
        BigInt divisor;
        while (1) {
            switch (TOKENTYPE token = next_token()) {
            case '*':
                res *= process_number();
                break;
            case '/':
                divisor = process_number();
                if (BigInt(0) == divisor) {
                    throw(CDivisionByZero());
                }
                res /= divisor;
                break;
            default:
                // возвращаемся с к низкоприоритетным.
                token_type = token;
                return res;
            }
        }

        return 0;
    }

    // Обработка числовых литералов
    BigInt process_number() {
        switch (next_token()) {
        case SUB:
            // поехали дальше за числом
            eat_number();
            return -number;
        case NUMBER:
            return number;
        }
        throw(CSyntaxError());
    }

    // Ожидаем именно численный литерал без знака
    void eat_number() {
        if (next_token() != NUMBER) throw(CSyntaxError());
    }

    // Следующий токен
    // возвращаем тип, число грузит в number.
    TOKENTYPE next_token() {
        // Если еще необработанный токен в «очереди» — выдаем его.
        if (token_type != UNDEF) {
            TOKENTYPE tmp = token_type;
            token_type = UNDEF;
            return tmp;
        }

        char ch = expression[pos];

        // Eating space
        while (ch == ' ') {
            ch = expression[++pos];
        }

        // Если число - забираем.
        int digitmaybe = ch - '0';
        if (0 <= digitmaybe && digitmaybe <= 9) {
            // Число.
            number = 0;
            do {
                number = number * 10 + digitmaybe;
                ch = expression[++pos];
                digitmaybe = ch - '0';
            } while (0 <= digitmaybe && digitmaybe <= 9);
            return NUMBER;
        }

        // Выделяем допустимые операции
        switch (ch) {
        case '\0':
        case '+':
        case '-':
        case '*':
        case '/':
            if (pos < strlen(expression)) pos++;
            return static_cast<TOKENTYPE>(ch);
        default:
            throw(CSyntaxError());
        }
    }
};

int main(int argc, char *argv[]) {
    enum ERROR_CODE { ERROR_SYNTAX_ERROR = 1, ERROR_DIVISION_BY_ZERO = 2 };

    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " [expression] " << std::endl;
        return 0; // Хотя возможно здесь надо  ERROR_SYNTAX_ERROR
    }

    CCalculator calc;
    try {
        BigInt result = calc.process(argv[1]);
        std::cout << result << std::endl;
    } catch (CSyntaxError &error_) {
        // Todo: красиво показывать позицию при ошибке синтаксиса.
        // Хотя, кому это надо.
        std::cout << "Syntax Error! Position " << calc.get_pos() + 1
                  << std::endl;
        return ERROR_SYNTAX_ERROR;
    } catch (CDivisionByZero &error_) {
        std::cout << "Division by zero! " << std::endl;
        return ERROR_DIVISION_BY_ZERO;
    }
    return 0;
}
