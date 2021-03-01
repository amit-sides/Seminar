import time
import sys
import os

def main():
    print("Hello! What's your name?")
    name = input()
    print(f"Hi, {name}!")
    print("What's your age?")
    age = input("age???")
    print(f"You are a boomer! {age}")
    print("What's your password?")
    password = input()
    print(f"bad one lol {password}")

    n = 1
    while True:
        print(n)
        n += 1
        time.sleep(1)
        if n==5:
            break


if __name__ == "__main__":
    main()
