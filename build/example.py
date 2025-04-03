import random
import time
import json
import os

class Item:
    def __init__(self, name, description, effect_type, effect_value):
        self.name = name
        self.description = description
        self.effect_type = effect_type
        self.effect_value = effect_value
    
    def use(self, creature):
        if self.effect_type == "health":
            creature.health += self.effect_value
            return f"{creature.name}'s health increased by {self.effect_value}!"
        elif self.effect_type == "experience":
            creature.experience += self.effect_value
            return f"{creature.name} gained {self.effect_value} experience!"

class MagicalCreature:
    def __init__(self, name, power, creature_type="Normal"):
        self.name = name
        self.power = power
        self.creature_type = creature_type
        self.max_health = 100
        self.health = self.max_health
        self.experience = 0
        self.level = 1
        self.power_level = 10
        self.hunger = 0
        self.happiness = 100
        self.inventory = []
        self.known_abilities = [power]
        self.evolution_stage = 1
    
    def speak(self):
        phrases = [
            f"I am {self.name}, master of {self.power}!",
            f"Beware my {self.power} powers!",
            f"Greetings, human. I possess the gift of {self.power}.",
            f"As a {self.creature_type} creature, I command {self.power}!",
            f"Level {self.level} {self.creature_type} at your service."
        ]
        return random.choice(phrases)
    
    def use_power(self):
        power_cost = random.randint(5, 15)
        if self.health >= power_cost:
            self.health -= power_cost
            self.hunger += 5
            exp_gain = random.randint(1, 5)
            self.experience += exp_gain
            self.check_level_up()
            return f"{self.name} uses {self.power}! Health: {self.health}/{self.max_health}, Experience: {self.experience}"
        else:
            return f"{self.name} is too tired to use {self.power}..."
    
    def feed(self, food_value=20):
        self.health = min(self.health + food_value, self.max_health)
        self.hunger = max(0, self.hunger - 15)
        self.happiness += 5
        return f"{self.name} has been fed. Health: {self.health}/{self.max_health}, Hunger: {self.hunger}"
    
    def rest(self):
        health_gain = random.randint(10, 25)
        self.health = min(self.health + health_gain, self.max_health)
        self.hunger += 10
        return f"{self.name} takes a nap and recovers {health_gain} health."
    
    def play(self):
        self.happiness += random.randint(10, 20)
        self.health -= random.randint(5, 10)
        exp_gain = random.randint(1, 3)
        self.experience += exp_gain
        return f"{self.name} plays happily! Happiness: {self.happiness}, Experience +{exp_gain}"
    
    def check_level_up(self):
        exp_needed = self.level * 20
        if self.experience >= exp_needed:
            self.level_up()
    
    def level_up(self):
        self.level += 1
        self.max_health += 10
        self.health = self.max_health
        print(f"\n🌟 {self.name} has reached level {self.level}! 🌟")
        
        if self.level % 5 == 0 and self.evolution_stage < 3:
            self.evolve()
    
    def evolve(self):
        self.evolution_stage += 1
        old_name = self.name
        
        if self.evolution_stage == 2:
            self.name = "Elder " + self.name
        else: 
            self.name = "Ancient " + old_name.replace("Elder ", "")
        
        print(f"\n⭐⭐⭐ EVOLUTION ⭐⭐⭐")
        print(f"{old_name} is evolving...")
        time.sleep(2)
        print(f"Say hello to {self.name}!")
    
    def status(self):
        return f"""
============ {self.name} Status ============
Type: {self.creature_type} (Evolution Stage: {self.evolution_stage})
Level: {self.level} (Experience: {self.experience})
Health: {self.health}/{self.max_health}
Power: {self.power} (Power Level: {self.power_level})
Hunger: {self.hunger}/100
Happiness: {self.happiness}/100
Known Abilities: {', '.join(self.known_abilities)}
Inventory Items: {len(self.inventory)}
==========================================
        """
    
    def add_to_inventory(self, item):
        self.inventory.append(item)
        return f"{item.name} has been added to {self.name}'s inventory!"
    
    def use_item(self, item_index):
        if 0 <= item_index < len(self.inventory):
            item = self.inventory.pop(item_index)
            return item.use(self)
        else:
            return "Invalid item index!"
    
    def show_inventory(self):
        if not self.inventory:
            return "Inventory is empty."
        
        inventory_text = f"\n===== {self.name}'s Inventory =====\n"
        for i, item in enumerate(self.inventory):
            inventory_text += f"{i+1}. {item.name}: {item.description}\n"
        
        return inventory_text
    
    def battle(self, opponent):
        print(f"\n⚔️ BATTLE START: {self.name} vs {opponent.name} ⚔️")
        time.sleep(1)
        
        while self.health > 0 and opponent.health > 0:
            # Replace integer division with regular division and int conversion
            player_damage = random.randint(5, 10) + int(self.power_level / 5)
            opponent.health -= player_damage
            print(f"{self.name} attacks with {self.power} and deals {player_damage} damage!")
            
            if opponent.health <= 0:
                print(f"\n{self.name} wins the battle!")
                return True, random.randint(10, 20)
                
            # Opponent attacks
            opponent_damage = random.randint(3, 8)
            self.health -= opponent_damage
            print(f"{opponent.name} attacks with {opponent.power} and deals {opponent_damage} damage!")
            
            if self.health <= 0:
                print(f"\n{opponent.name} wins the battle!")
                self.health = 1 
                return False, random.randint(5, 10)
            
            time.sleep(1)

class FireCreature(MagicalCreature):
    def __init__(self, name, power):
        super().__init__(name, power, "Fire")
        self.power_level += 5  

class WaterCreature(MagicalCreature):
    def __init__(self, name, power):
        super().__init__(name, power, "Water")
        self.max_health += 15 
        self.health = self.max_health

def generate_random_item():
    item_types = [
        {"name": "Health Potion", "description": "Restores health points", "effect_type": "health", "value_range": (15, 30)},
        {"name": "Experience Orb", "description": "Grants additional experience", "effect_type": "experience", "value_range": (10, 25)}
    ]
    
    item_type = random.choice(item_types)
    effect_value = random.randint(*item_type["value_range"])
    
    return Item(item_type["name"], item_type["description"], item_type["effect_type"], effect_value)

def generate_creature(predefined=False):
    names = ["Zephyr", "Lumina", "Frost", "Ember", "Shadow", "Terra", "Aqua", "Ventus", 
             "Phoenix", "Luna", "Solaris", "Gaia", "Tempest", "Aurora"]
    
    powers = ["fire", "water", "earth", "air", "lightning", "ice", "darkness", "light"]
    
    if predefined:
        name = random.choice(names)
        power = random.choice(powers)
        
        if power in ["fire", "lightning"]:
            return FireCreature(name, power)
        elif power in ["water", "ice"]:
            return WaterCreature(name, power)
        else:
            return MagicalCreature(name, power)
    else:
        return MagicalCreature(random.choice(names), random.choice(powers))

def main():
    print("🌟 Welcome to the Magical Creature Simulator 🌟")
    print("=" * 50)
    print("\nSelect your creature type:")
    print("1. Fire Creature")
    print("2. Water Creature")
    print("3. Regular Creature")
    
    choice = input("Enter your choice (1-3): ")
    
    if choice == '1':
        creature = FireCreature("Ember", "fire")
    elif choice == '2':
        creature = WaterCreature("Aqua", "water")
    else:
        creature = generate_creature()
    
    print(f"A wild {creature.name} appears!")
    print(creature.speak())
    
    day = 1
    while creature.health > 0:
        print(f"\n--- Day {day} ---")
        print("\nWhat would you like to do?")
        print("1. Let the creature speak")
        print("2. Ask the creature to use its power")
        print("3. Feed the creature")
        print("4. Let the creature rest")
        print("5. Play with the creature")
        print("6. View creature status")
        print("7. View/manage inventory")
        print("8. Go exploring")
        print("9. Battle another creature")
        print("10. Advance to next day")
        print("11. Exit")
        
        choice = input("Enter your choice (1-11): ")
        
        if choice == '1':
            print(creature.speak())
        elif choice == '2':
            print(creature.use_power())
        elif choice == '3':
            print(creature.feed())
        elif choice == '4':
            print(creature.rest())
        elif choice == '5':
            print(creature.play())
        elif choice == '6':
            print(creature.status())
        elif choice == '7':
            print(creature.show_inventory())
            if creature.inventory:
                use_item = input("Would you like to use an item? (y/n): ").lower()
                if use_item == 'y':
                    item_index = int(input(f"Enter item number (1-{len(creature.inventory)}): ")) - 1
                    print(creature.use_item(item_index))
        elif choice == '8':
            print(f"\n{creature.name} goes exploring...")
            time.sleep(2)
            
            event = random.random()
            
            if event < 0.4: 
                item = generate_random_item()
                print(f"{creature.name} found a {item.name}!")
                print(creature.add_to_inventory(item))
            elif event < 0.7: 
                exp_gain = random.randint(5, 15)
                creature.experience += exp_gain
                print(f"{creature.name} learned something new! Experience +{exp_gain}")
                creature.check_level_up()
            else: 
                damage = random.randint(5, 15)
                creature.health -= damage
                print(f"{creature.name} got injured during exploration! Health -{damage}")
        elif choice == '9':
            opponent = generate_creature(True)
            print(f"A wild {opponent.name} appears!")
            
            battle_choice = input(f"Do you want {creature.name} to battle? (y/n): ").lower()
            
            if battle_choice == 'y':
                won, exp_gain = creature.battle(opponent)
                if won:
                    creature.experience += exp_gain
                    print(f"{creature.name} gained {exp_gain} experience!")
                    creature.check_level_up()
        elif choice == '10':
            day += 1
            creature.hunger += 15
            creature.happiness = max(0, creature.happiness - 10)
            
            heal_amount = random.randint(5, 15)
            creature.health = min(creature.health + heal_amount, creature.max_health)
            print(f"A new day dawns. {creature.name} recovered {heal_amount} health overnight.")
        elif choice == '11':
            print(f"Farewell! {creature.name} waves goodbye!")
            break
        else:
            print("Invalid choice, please try again.")
        
        if choice in ['2', '8', '9']:
            time.sleep(1)

if __name__ == "__main__":
    main()