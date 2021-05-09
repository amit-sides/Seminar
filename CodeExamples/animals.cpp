#include <string>
#include <iostream>

class Animal {
protected:
    std::string name;
    int         age;
public:
    Animal(std::string name, int age)
    {
        this->name = name;
        this->age = age;
        std::cout << "My name is " << this->name << " and I'm ";
        std::cout << this->age << " years old." << std::endl;
    }

    virtual void make_sound() = 0;
};

class Dog: public Animal {
    using Animal::Animal; // Use constructor of Animal
public:
    void make_sound() override
    {
        std::cout << this->name << " Barks!" << std::endl;
    }
};

class Duck: public Animal {
    using Animal::Animal; // Use constructor of Animal
public:
    void make_sound() override
    {
        std::cout << this->name << " Quacks!" << std::endl;
    }
};

int main() {
    Dog dog = Dog("Rexy", 5);
    Duck duck = Duck("Nini", 3);
    dog.make_sound();
    duck.make_sound();
    return 0;
}
