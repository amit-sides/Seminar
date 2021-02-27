import time

def main():
    print("Hello! I will count for you:")

    n = 1
    while True:
        print(n)
        n += 1
        time.sleep(1)
        if n==100:
            break


if __name__ == "__main__":
    main()
