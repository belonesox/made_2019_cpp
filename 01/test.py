#!/usr/bin/env python
# -*- coding: utf-8 -*-
# pylint: disable=W0107, C0330, C0301, W0621, E1101
'''
Будем использовать TDD.
Сначала напишем тесты, используя как mock стандартный калькулятор bc, 
как mock тестируемой программы, и сам python для расчета (нет времени считать самому).
'''
from __future__ import print_function
from functools import update_wrapper
import sys
import subprocess
import random
import math

# Расширяемое количество правильных выражений сделаем потом генерацией вероятностных тестов.
good_expressions = """
-515/219*  140
2 + 3 * 4 - -2
2 + 3
5 + -6 
7+6+7+8-5
-724+     627/      -66-609*   -466+  953* -591*   696-     -723-      -624+707- -293
"""
# 65535*65535-65535*23
# 2147483647+2147483647-2147483647*2

# тут конечно да, сложность что bc сьест больше, чем нам надо. Но часть ложных можно и тут.
fail_expessions = """
44 d ddd+5666
----6:778
--6+
+
-
*
/
/456
*456
--33
123/
124*
-456-
567-
678+
12/0
0/-0
-0/0
"""

# Тут внутри хак-код, чтобы компенсировать классическую проблему
# Python, в котором целочисленное деление суть floor division, см. https://www.python.org/dev/peps/pep-0238/
# Для тех, кто не в курсе, то -5//2=-3
# Т.е. сам интерпретатор python, c операторами «/» и «//» нельзя использовать для тестов.
# А хотелось бы иметь для тестирования и его и линуксовый «bc», и сгенерированный «calc»
# Для этого хака есть пакет, но по условиям квеста ничего нельзя ставить.
# Его можно было бы отсадить в отдельный файл, но лишний файл это тоже некрасиво, плюс тут проблема разного импорта
# релятивно расположенных модулей в python2 и python3, а файл хотелось сделать универсально запускаемым.
# Поэтому просто промотайте эту магию, до следующего блока комментариев.


def lname(op):
    """The left name of an op"""
    return '__' + op + '__'


def rname(op):
    """The right name of an op, switches shifts"""
    shifts = set(['lshift', 'rshift'])
    if op in shifts:
        op = list(shifts - set([op]))[0]
    return '__r' + op + '__'


class base_infix(object):
    """The base infix class"""

    op = ()  # The operations to use

    def __init__(self, function):
        """Creates the decorated function"""
        self._function = function
        update_wrapper(self, self._function)
        ldict = dict()
        rdict = dict()
        for op in self.op:
            ldict[lname(op)] = lbind.__call__
            rdict[rname(op)] = rbind.__call__

        self.lbind = type('_'.join(self.op)+'_lbind', (lbind,), ldict)
        self.rbind = type('_'.join(self.op)+'_rbind', (rbind,), rdict)

    @property
    def __call__(self):
        """Wraps self"""
        return self._function

    def left(self, other):
        """Returns a partially applied infix operator"""
        return self.rbind(self._function, other)

    def right(self, other):
        return self.lbind(self._function, other)


class rbind(object):
    def __init__(self, function, binded):
        self._function = function
        update_wrapper(self, self._function)
        self.binded = binded

    def __call__(self, other):
        return self._function(other, self.binded)

    def reverse(self, other):
        return self._function(self.binded, other)

    def __repr__(self):
        return "<{0.__class__.__name__}: Waiting for left side>".format(self)


class lbind(object):
    def __init__(self, function, binded):
        self._function = function
        update_wrapper(self, self._function)
        self.binded = binded

    def __call__(self, other):
        return self._function(self.binded, other)

    def reverse(self, other):
        return self._function(other, self.binded)

    def __repr__(self):
        return "<{0.__class__.__name__}: Waiting for right side>".format(self)


def make_infix(*ops):
    """Returns a custom infix type, using the given operation."""
    ops = list(ops)
    if 'div' in ops:
        ops += ['truediv']
    opdict = dict(op=ops)
    for op in ops:
        opdict[lname(op)] = base_infix.left
        opdict[rname(op)] = base_infix.right
    return type('_'.join(ops)+'_infix', (base_infix,), opdict)


div_infix = make_infix('div')


@div_infix
def div(x, y):
    return abs(x)//abs(y)*int(math.copysign(1, x)*math.copysign(1, y))


def have_bc():
    '''
    Есть ли на системе калькулятор BC.
    '''
    # красиво было бы shutil.which использовать, но он только в третьем питоне.    
    try:
        subprocess.call(["bc", "2+2"])
        return True
    except OSError as e:
        if e.errno == errno.ENOENT:
            return False
        else:
            # Something else went wrong while trying to run `wget`
            return False




# Все, магия кончилась, выше мы завели «псевдооператор» «/div/» с правильной семантикой и приоритетом.


def run_mock(expression):
    '''
    Запуск эмулятора калькулятора.
    Используется для тестирования тестов, для тестирования самого калькулятора не обязателен,
    хотя под всеми линукс-системами bc есть.
    '''
    p = subprocess.Popen("bc", stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate(("scale=0\n" + expression + '\n').encode("utf-8"))
    returncode = p.returncode
    result = None
    if not err:
        result = int(round(float(out.strip())))
    else:
        returncode = 1
    return result, returncode


def run_calc(expression):
    p = subprocess.Popen(["./calc", expression], stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate()
    # print(expression.encode("utf-8"))
    # print("out", out)
    # print("err", err)
    returncode = p.returncode  # bc always returns zero
    result = None
    if not returncode:
        result = int(round(float(out.strip())))
    return result, returncode


def test(func2test=run_mock):
    print("*"*50)
    print("Testing ",  func2test.__name__)

    def test_good_expression(expression):
        print("     expression  ", expression)
        result, returncode = func2test(expression)
        if returncode != 0:
            print("!"*10, " returncode is not zero! ", returncode)
            return False
        python_expression = expression.replace('/', ' /div/ ')
        expecting = eval(python_expression)
        # print("###", python_expression)
        re_ = int(round(expecting))
        res_ = int(round(result))
        if (res_ != re_ and abs(re_) > 0) and abs((res_ - re_)/re_) > 1e-6:
            print("!"*5, "«", expression, "» returned ", res_, " expected ", re_)
            return False
        return True

    print("     Testing good expressions ")
    for expression in good_expressions.strip().split("\n"):
        if not test_good_expression(expression):
            return False

    print("     Testing bad expressions ")
    for expression in fail_expessions.strip().split("\n"):
        print("     expression  ", expression)
        result, returncode = func2test(expression)
        if returncode == 0:
            print("!"*10, " returncode shoud be nonzero! ")
            return False

    print("     Testing random good expressions ")
    max_integer = 1024
    # max_integer = 64
    if func2test != run_mock:
        # Врубаем тест на полную. В bc вроде выходило иногда переполнение, надо еще погонять.
        max_integer = 2147483647  # — big probablility of overfloating
    for k in range(1000):  # сколько угодно тестов
        len_of_expression = random.randint(1, 20)
        terms_expr = [random.randint(1, max_integer)
                      for i in range(len_of_expression)]
        signs_expr = [random.choice(['', '-'])
                      for i in range(len_of_expression)]
        ops_expr = [random.choice(['+', '-', '*', '/'])
                    for i in range(len_of_expression-1)]
        expr_terms = []
        for k, (s, t) in enumerate(zip(signs_expr, terms_expr)):
            expr_terms.append(str(s)+str(t))
            if k < len(ops_expr):
                expr_terms.append(ops_expr[k])
            expr_terms.append(' '*random.randint(0, 6))
        expr = ''.join(expr_terms)
        expr = expr.replace('--', '-')
        if not test_good_expression(expr):
            return False

    print("Testing ",  func2test.__name__, 'done')
    print("-"*50)
    return True


if __name__ == "__main__":
    if have_bc():
        # если в системе есть «bc», сначала потестируем сами тесты и питон.
        print("*"*50)
        print("Selftesting")
        print("-"*50)
        if not test():
            sys.exit(-1)
        # ну значит с тестами скорее всего ОК        
    
    print("*"*50)
    print("Testing of real program")
    print("-"*50)
    # Тестируем нашу программу
    if not test(run_calc):
        sys.exit(-1)
    pass
