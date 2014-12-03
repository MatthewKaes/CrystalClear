# Inside of a python script.
# Call this function with "fibonacci"
# in Crystal Clear.

def fib(n):
     a, b, count = 0, 1, 0
     while count < n:
         print(a, end=' ')
         a, b = b, a+b
         count = count + 1
     print()
	 
def fibonacci():
     fib(10)