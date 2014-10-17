def fib(n):
     a, b, count = 0, 1, 0
     while count < n:
         print(a, end=' ')
         a, b = b, a+b
         count = count + 1
     print()
fib(10)