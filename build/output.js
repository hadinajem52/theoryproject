// JavaScript code generated from Python

const x = 10;
const y = 20;
const z = (x + (y * 2));
if ((x > 5)) {
  console.log("x is greater than 5");
} else if ((x === 5)) {
  console.log("x equalsssssssssssssssssssssssss 5");
} else {
  console.log("x is lesssssssssssssssssssssssssssssss than 5");
}
for (let i of Array.from({length: 5}, (_, i) => i)) {
  console.log(`Looooooooooooooooooooooooooooooooooooooop iteration ${i}`);
}
const counter = 0;
while ((counter < 3)) {
  console.log(`While loop: ${counter}`);
  const counter = (counter + 1);
}
function greet(name, greeting = "Hello") {
  return `${greeting}, ${name}!`;
}


console.log(greet("World"));
console.log(greet("Python", "Welcome to"));
const numbers = [1, 2, 3, 4, 5];
const doubled = numbers.map(num => (num * 2));
console.log(doubled);
const person = {"name": "HAdi", "age": 30, "languages": ["Python", "JavaScript", "C++"]};
console.log(`Person: ${person['name']}, Age: ${person['age']}`);
class Animal {
  constructor(name) {
    this.name = name;
  }

  speak() {
    console.log(`${self.name} makes a sound like a human`);
  }

}

class Dog extends Animal {
  speak() {
    console.log(`${self.name} barks like a human`);
  }

}

const fido = new Dog("Fido is a dog");
fido.speak();
try {
  const result = (10 / 0);
} catch (e) {
  if (!(e instanceof Error)) {
    throw e; // Re-throw if not the right type
  }
  console.log("Cannot divide by zero");
} finally {
  console.log("This always executes");
}
