from abc import ABC, abstractmethod

class Animal(ABC):  # Inheritance from ABC - the built-in abstract class
    def __init__(self, name, age):
        self.name = name
        self.age = age
        print(f"My name is {name} and I'm {age} years old.")

    @abstractmethod
    def make_sound(self):
        pass

class Dog(Animal):
    def make_sound(self):
        print(f"{self.name} Barks!")

class Duck(Animal):
    def make_sound(self):
        print(f"{self.name} Quacks!")

def main():
    dog = Dog("Rexi", 5)
    duck = Duck("Nini", 3)
    dog.make_sound()
    duck.make_sound()


if __name__ == '__main__':
    main()
