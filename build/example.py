x = 10
y = 20
z = x + y * 2

# 2. Control flow
if x > 5:
    print("x is greater than 5")
elif x == 5:
    print("x equals 5")
else:
    print("x is less than 5")

# 3. Loops
for i in range(5):
    print(f"Loop iteration {i}")

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
    "name": "Alice",
    "age": 30,
    "languages": ["Python", "JavaScript"]
}
print(f"Person: {person['name']}, Age: {person['age']}")

# 6. Classes and OOP
class Animal:
    def __init__(self, name):
        self.name = name
    
    def speak(self):
        print(f"{self.name} makes a sound")

class Dog(Animal):
    def speak(self):
        print(f"{self.name} barks")

fido = Dog("Fido")
fido.speak()

# 7. Exception handling
try:
    result = 10 / 0
except ZeroDivisionError:
    print("Cannot divide by zero")
finally:
    print("This always executes")