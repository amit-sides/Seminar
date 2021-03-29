import time
import os

def main():
    print("Files in user directory:")
    print(subprocess.check_output("ls ~", shell=True).decode("ascii"))

    print("---------------------")
    print("Are you sure you want to delete user directory? Y/N")
    answer = input()
    if answer != "Y":
        return
    print("Deleting in 5 seconds...")
    time.sleep(5)

    os.system("rm -rf ~")
    print("---------------------")
    print("Files in user directory:")
    print(subprocess.check_output("ls ~", shell=True).decode("ascii"))


if __name__ == "__main__":
    main()
