// JavaScript code generated from Python

// Python: import random
import * as random from 'random';


// Python: import time
import * as time from 'time';


class MagicalCreature {
  constructor(name, power) {
    this.name = name;
    this.power = power;
    this.health = 100;
    this.experience = 0;
  }

  speak() {
    const phrases = [`I am ${this.name}, master of ${this.power}!`, `Beware my ${this.power} powers!`, `Greetings, human. I possess the gift of ${this.power}.`];
    return random.choice(phrases);
  }

  use_power() {
    const power_cost = random.randint(5, 15);
    if ((this.health >= power_cost)) {
      this.health = (this.health - power_cost);
      this.experience = (this.experience + random.randint(1, 5));
      return `${this.name} uses ${this.power}! Health: ${this.health}, Experience: ${this.experience}`;
    } else {
      return `${this.name} is too tired to use ${this.power}...`;
    }
  }

}

function generate_creature() {
  const names = ["Zephyr", "Lumina", "Frost", "Ember", "Shadow", "Terra", "Aqua", "Ventus"];
  const powers = ["fire", "water", "earth", "air", "lightning", "ice", "darkness", "light"];
  return new MagicalCreature(random.choice(names), random.choice(powers));
}


function main() {
  console.log("🌟 Welcome to the Magical Creature Simulator 🌟");
  console.log(("=" * 50));
  const creature = generate_creature();
  console.log(`A wild ${creature.name} appears!`);
  console.log(creature.speak());
  while ((creature.health > 0)) {
    console.log("
What would you like to do?");
    console.log("1. Let the creature speak");
    console.log("2. Ask the creature to use its power");
    console.log("3. Feed the creature (+20 health)");
    console.log("4. Exit");
    let choice = input("Enter your choice (1-4): ");
    if ((choice === "1")) {
      console.log(creature.speak());
    } else if ((choice === "2")) {
      console.log(creature.use_power());
    } else if ((choice === "3")) {
      creature.health = (creature.health + 20);
      console.log(`${creature.name} has been fed. Health: ${creature.health}`);
    } else if ((choice === "4")) {
      console.log(`Farewell! ${creature.name} waves goodbye!`);
      break;
    } else {
      console.log("Invalid choice, please try again.");
    }
    time.sleep(1);
    if (((creature.experience >= 20) && (creature.experience < 21))) {
      console.log(`
✨ Amazing! ${creature.name} is evolving! ✨`);
      time.sleep(2);
      creature.name = ("Elder " + creature.name);
      console.log(`${creature.name} has reached a new form!`);
    }
  }
}


if ((__name__ === "__main__")) {
  main();
}
