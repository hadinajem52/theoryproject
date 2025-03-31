// JavaScript code generated from Python

const x = 10;
const y = 20;
const z = (x + (y * 2));
if ((x > 5)) {
  console.log("x is greater than 5");
} else if ((x === 5)) {
  console.log("x equals 5");
} else {
  console.log("x is less than 5");
  for (let i of /* Use Array(n).fill().map((_, i) => i) */(5)) {
    console.log(`Loop iteration ${i}`);
    const counter = 0;
    while ((counter < 3)) {
      console.log(`While loop: ${counter}`);
      const counter = (counter + 1);
      function greet(name, greeting = "Hello") {
        return `${greeting}, ${name}!`;
        console.log(greet("World"));
        console.log(greet("Python", "Welcome to"));
        const numbers = [1, 2, 3, 4, 5];
        const doubled = /* Unknown expression */;
        console.log(doubled);
        const person = {"name": "Alice", "age": 30, "languages": ["Python", "JavaScript"]};
        console.log(`Person: ${person['name']}, Age: ${person['age']}`);
        class Animal {
          constructor(name) {
            this.name = name;
            speak() {
              console.log(`${self.name} makes a sound`);
              class Dog extends Animal {
                constructor() {
                }
                speak() {
                  console.log(`${self.name} barks`);
                  const fido = Dog("Fido");
                  fido.speak();
                  try {
                    const result = (10 / 0);
                  } catch (e) {
                    if (!(e instanceof ZeroDivisionError)) {
                      throw e; // Re-throw if not the right type
                    }
                    console.log("Cannot divide by zero");
                  } finally {
                    console.log("This always executes");
                  }
                }

              }

            }

          }

        }

      }

    }
  }
}
