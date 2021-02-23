import time

def main():
    print("Hello, World!")
    time.sleep(30)
    name = input("What is your name?\n")
    print(f"Hello, {name}!")

    time.sleep(50)
    print("Goodbye!")


if __name__ == "__main__":
    main()
