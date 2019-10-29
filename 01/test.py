#!/usr/bin/env python
# -*- coding: utf-8 -*-
#pylint: disable=W0107, C0330, C0301, W0621, E1101
'''
Будем использовать TDD.
Сначала напишем тесты, используя как mock стандартный калькулятор bc, 
как mock тестируемой программы, и сам python для расчета (нет времени считать самому).
'''
from __future__ import print_function
import sys
import subprocess
import random

# Расширяемое количество правильных выражений сделаем потом генерацией вероятностных тестов.
good_expressions = """
2 + 3 * 4 - -2
2 + 3
5 + -6 
7+6+7+8-5
65535*65535-65535*23
2147483647+2147483647-2147483647*2
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
    out, err = p.communicate(("scale=30\n" + expression + '\n').encode("utf-8"))
    # print(expression.encode("utf-8"))
    # print("out", out)
    # print("err", err)
    # returncode = p.returncode  # bc always returns zero
    returncode = p.returncode
    result = None
    if not err:
        result = int(round(float(out.strip())))     
    else:
        returncode = 1    
    return result, returncode

def test(func2test=run_mock):
    print("*"*50)
    print("Testing ",  func2test.__name__)

    print("     Testing random good expressions ")

    def test_good_expression(expression):
        print("     expression  ", expression)
        result, returncode = func2test(expression)
        if returncode != 0:
            print("!"*10, " returncode is not zero! ")
            return False
        expecting = eval(expression)
        re_ = int(round(expecting))
        res_ = int(round(result))
        if (res_ != re_ and abs(re_)>0) and abs((res_ - re)/re_)>1e-6:
            print("!"*5, " returned ", res_, " expected ", re_)
            return False
        return True


    max_integer = 1024
    # max_integer = 2147483647 — big probablility of overfloating 
    for k in range(1000): # сколько угодно тестов
        len_of_expression = random.randint(1, 20)
        terms_expr  = [random.randint(1, max_integer) for i in range(len_of_expression)]
        signs_expr  = [random.choice(['', '-']) for i in range(len_of_expression)]
        ops_expr  = [random.choice(['+', '-', '*', '/']) for i in range(len_of_expression-1)]
        expr_terms = []
        for k,(s,t) in enumerate(zip(signs_expr, terms_expr)):
            expr_terms.append(str(s)+str(t))    
            if k<len(ops_expr):
                expr_terms.append(ops_expr[k])    
            expr_terms.append(' '*random.randint(0, 6))        
        expr = ''.join(expr_terms)
        expr = expr.replace('--', '-')
        if not test_good_expression(expr):
            return False

    print("     Testing good expressions ")
    for expression in good_expressions.strip().split("\n"):
        if not test_good_expression(expression):
            return False

    print("     Testing bad expressions ")
    for expression in fail_expessions.strip().split():
        print("     expression  ", expression)
        result, returncode = func2test(expression)
        if returncode == 0:
            print("!"*10, " returncode shoud be nonzero! ")
            return False

    print("Testing ",  func2test.__name__, 'done')
    print("-"*50)
    return True


if __name__ == "__main__":
    print("*"*50)
    print("SelfTesting")
    print("-"*50)
    if not test():
        sys.exit(-1)
    pass        

