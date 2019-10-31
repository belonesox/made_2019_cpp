#include <cassert>
#include <climits>
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

enum TOKENTYPE {
    UNDEF = 0,
    EOL = '\0',
    NUMBER = 1,
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = '/'
};

enum ERROR_CODE { ERROR_OK, ERROR_SYNTAX_ERROR, ERROR_DIVISION_BY_ZERO };

class CCalculator {

  public:
    // в процессе парсинга значения числовых литералов, в конце — результат.
    BigInt number;

    // Основной интерфейс.
    ERROR_CODE process(const char *input_expression) {
        assert(input_expression);
        error_code = ERROR_OK;
        number = 0;
        token_type = UNDEF;
        expression = input_expression;
        pos = 0;

        try {
            number = process_low_precendence();
        } catch (ERROR_CODE error_) {
            return error_;
        }
        return ERROR_OK;
    }

    // Код результата
    ERROR_CODE error_code;

    // Текущая позиция, публична, чтобы знать, где произошла ошибка разбора
    int pos;

  private:
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
                throw(ERROR_SYNTAX_ERROR);
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
                    throw(ERROR_DIVISION_BY_ZERO);
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
        throw(ERROR_SYNTAX_ERROR);
    }

    // Ожидаем именно численный литерал без знака
    void eat_number() {
        if (next_token() != NUMBER) throw(ERROR_SYNTAX_ERROR);
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
            throw(ERROR_SYNTAX_ERROR);
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cout << "Usage: " << argv[0] << " [expression] " << std::endl;
        return 0;
    }

    CCalculator calc;
    ERROR_CODE err_cod = calc.process(argv[1]);

    if (err_cod == ERROR_OK) {
        std::cout << calc.number << std::endl;
        return 0;
    }

    switch (err_cod) {
    case ERROR_SYNTAX_ERROR:
        // Todo: красиво показывать позицию при ошибке синтаксиса.
        // Хотя, кому это надо.
        std::cout << "Syntax Error! Position " << calc.pos + 1 << std::endl;
        break;
    case ERROR_DIVISION_BY_ZERO:
        std::cout << "Division by zero! " << std::endl;
        break;
    default:
        assert(0);
    }
    return err_cod;
}
