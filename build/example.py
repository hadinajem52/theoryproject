import random
import time

class MagicalCreature:
    def __init__(self, name, power):
        self.name = name
        self.power = power
        self.health = 100
        self.experience = 0
    
    def speak(self):
        phrases = [
            f"I am {self.name}, master of {self.power}!",
            f"Beware my {self.power} powers!",
            f"Greetings, human. I possess the gift of {self.power}."
        ]
        return random.choice(phrases)
    
    def use_power(self):
        power_cost = random.randint(5, 15)
        if self.health >= power_cost:
            self.health -= power_cost
            self.experience += random.randint(1, 5)
            return f"{self.name} uses {self.power}! Health: {self.health}, Experience: {self.experience}"
        else:
            return f"{self.name} is too tired to use {self.power}..."

def generate_creature():
    names = ["Zephyr", "Lumina", "Frost", "Ember", "Shadow", "Terra", "Aqua", "Ventus"]
    powers = ["fire", "water", "earth", "air", "lightning", "ice", "darkness", "light"]
    return MagicalCreature(random.choice(names), random.choice(powers))

def main():
    print("🌟 Welcome to the Magical Creature Simulator 🌟")
    print("=" * 50)
    
    # Create a magical creature
    creature = generate_creature()
    print(f"A wild {creature.name} appears!")
    print(creature.speak())
    
    # Interactive loop
    while creature.health > 0:
        print("\nWhat would you like to do?")
        print("1. Let the creature speak")
        print("2. Ask the creature to use its power")
        print("3. Feed the creature (+20 health)")
        print("4. Exit")
        
        choice = input("Enter your choice (1-4): ")
        
        if choice == '1':
            print(creature.speak())
        elif choice == '2':
            print(creature.use_power())
        elif choice == '3':
            creature.health += 20
            print(f"{creature.name} has been fed. Health: {creature.health}")
        elif choice == '4':
            print(f"Farewell! {creature.name} waves goodbye!")
            break
        else:
            print("Invalid choice, please try again.")
        
        # Add some suspense
        time.sleep(1)
        
        # Check if creature evolved
        if creature.experience >= 20 and creature.experience < 21:
            print(f"\n✨ Amazing! {creature.name} is evolving! ✨")
            time.sleep(2)
            creature.name = "Elder " + creature.name
            print(f"{creature.name} has reached a new form!")

if __name__ == "__main__":
    main()