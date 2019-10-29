#!/usr/bin/env python
# -*- coding: utf-8 -*-
#pylint: disable=W0107, C0330, C0301, W0621, E1101
'''
Будем использовать TDD.
Сначала напишем тесты, используя как mock стандартный калькулятор bc, 
как mock тестируемой программы, и сам python для расчета (нет времени считать самому).
'''
from __future__ import print_function
import subprocess

# Расширяемое количество правильных выражений сделаем потом генерацией вероятностных тестов.
good_expressions = """
2 + 3 * 4 - -2
2 + 3
5 + -6 
7+6+7+8-5
"""

#тут конечно да, сложность что bc сьест больше, чем нам надо. Но часть ложных можно и тут.
fail_expessions = """
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
"""

def run_mock(expression):
    p = subprocess.Popen("bc", stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = p.communicate((expression + '\n').encode("utf-8"))
    # print(expression.encode("utf-8"))
    # print("out", out)
    # print("err", err)
    # returncode = p.returncode  # bc always returns zero
    returncode = p.returncode
    result = None
    if not err:
        result = int(out.strip())     
    else:
        returncode = 1    
    return result, returncode

def test(func2test=run_mock):
    print("*"*50)
    print("Testing ",  func2test.__name__)

    print("     Testing good expressions ")
    for expression in good_expressions.strip().split("\n"):
        print("     expression  ", expression)
        result, returncode = func2test(expression)
        if returncode != 0:
            print("!"*10, " returncode is not zero! ")
        expecting = eval(expression)
        if result != expecting:
            print("!"*5, " returned ", result, " expected ", expecting)

    print("     Testing bad expressions ")
    for expression in fail_expessions.strip().split():
        print("     expression  ", expression)
        result, returncode = func2test(expression)
        if returncode == 0:
            print("!"*10, " returncode shoud be nonzero! ")

    print("Testing ",  func2test.__name__, 'done')
    print("-"*50)

if __name__ == "__main__":
    print("*"*50)
    print("SelfTesting")
    print("-"*50)
    test()

