// JavaScript code generated from Python

let x = 10;
let y = 20;
let z = (x + (y * 2));
if ((x > 5)) {
  console.log("x is greater than 5");
} else if ((x === 5)) {
  console.log("x equals 5");
} else {
  console.log("x is less than 5");
  for (let i of /* Use Array(n).fill().map((_, i) => i) */(5)) {
    console.log(`Loop iteration ${i}`);
    let counter = 0;
    while ((counter < 3)) {
      console.log(`While loop: ${counter}`);
      let counter = (counter + 1);
      function greet(name, greeting = "Hello") {
        return `${greeting}, ${name}!`;
        console.log(greet("World"));
        console.log(greet("Python", "Welcome to"));
        let numbers = [1, 2, 3, 4, 5];
        let doubled = /* Unknown expression */;
        console.log(doubled);
        let person = {"name": "Alice", "age": 30, "languages": ["Python", "JavaScript"]};
        console.log(`Person: ${person['name']}, Age: ${person['age']}`);
        class Animal {
          __init__(, name) {
            this.name = name;
            function speak() {
              console.log(`${self.name} makes a sound`);
              class Dog extends Animal {
                constructor() {
                }
                function speak() {
                  console.log(`${self.name} barks`);
                  let fido = Dog("Fido");
                  fido.speak();
                  // Unknown node type
                }

              }

            }

          }

        }

      }

    }
  }
}
