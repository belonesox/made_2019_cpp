HW4: Матрица

Нужно написать класс-матрицу, тип элементов `int`.

В конструкторе задается количество рядов и строк.

Поддерживаются операции:

- получить
  - количество строк(rows)
  - столбцов(columns),
  - конкретный элемент,
- умножить на число (\*=),
- сравнение на равенство/неравенство.

В случае ошибки выхода за границы бросать исключение:
throw std::out_of_range("")

Пример:

```cpp
const size_t rows = 5;
const size_t cols = 3;

Matrix m(rows, cols);

assert(m.getRows() == 5);
assert(m.getColumns() == 3);

m[1][2] = 5; // строка 1, колонка 2
double x = m[4][1];

m *= 3; // умножение на число

Matrix m1(rows, cols);

if (m1 == m)
{
}

```

Подсказка:

Чтобы реализовать семантику [][] понадобится прокси-класс.

Оператор матрицы возращает другой класс, в котором тоже используется оператор [] и уже этот класс возвращает значение.
