x = 10
y = 20
z = x + y * 2

# 2. Control flow
if x > 5:
    print("x is greater than 5")
elif x == 5:
    print("x equalsssssssssssssssssssssssss 5")
else:
    print("x is lesssssssssssssssssssssssssssssss than 5")

# 3. Loops
for i in range(5):
    print(f"Looooooooooooooooooooooooooooooooooooooop iteration {i}")

counter = 0
while counter < 3:
    print(f"While loop: {counter}")
    counter += 1

# 4. Functions
def greet(name, greeting="Hello"):
    return f"{greeting}, {name}!"

print(greet("World"))
print(greet("Python", "Welcome to"))

# 5. Data structures
numbers = [1, 2, 3, 4, 5]
doubled = [num * 2 for num in numbers]
print(doubled)

person = {
    "name": "HAdi",
    "age": 30,
    "languages": ["Python", "JavaScript", "C++"]
}
print(f"Person: {person['name']}, Age: {person['age']}")

# 6. Classes and OOP
class Animal:
    def __init__(self, name):
        self.name = name
    
    def speak(self):
        print(f"{self.name} makes a sound like a human")

class Dog(Animal):
    def speak(self):
        print(f"{self.name} barks like a human")

fido = Dog("Fido is a dog")
fido.speak()

# 7. Exception handling
try:
    result = 10 / 0
except ZeroDivisionError:
    print("Cannot divide by zero")
finally:
    print("This always executes")