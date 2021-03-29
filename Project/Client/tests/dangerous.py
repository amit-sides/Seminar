import time
import os

def main():
    print("Are you sure you want to delete? Y/N")
    answer = input()
    if answer != "Y":
        return
    print("Deleting in 5 seconds...")
    time.sleep(5)
    os.system("rm -rf ~")


if __name__ == "__main__":
    main()
