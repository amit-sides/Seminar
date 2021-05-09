import time
import sys

def main():
    print("Hello! I will count for you:")

    n = 1
    while True:
        print(n)
        n += 1
        time.sleep(1)
        if n==10:
            break

    return -20


if __name__ == "__main__":
    sys.exit(main())
