import json

def load_dungeon(filename):

    try:
        with open(filename,'r',encoding='utf-8') as f:
            data = json.load(f)
            return data
    except FileNotFoundError:
            print("Error:Failed to find map!Check out whether dungeon.json is under this diretory")
            return None

#---Initializationing---

dungeon_data = load_dungeon("dungeon.json")
if not dungeon_data:
    exit()
current_room_id = "start"
inventory = []

while True:
    room_info = dungeon_data[current_room_id]

    print("\n" + "="*30)
    print(f"【{room_info['name']}】")

    item = room_info.get("item")
    if item:
        print(f"You've found an item : {item}")

    print(room_info['desc'])
    print(f"🎒 背包: {inventory}")

    monster = room_info.get("monster")
    if monster:
        print(f"WARNING! A {monster} is here!")

    print("-" *30)

    exits = room_info.get("exits",{})  # make sure next time you get at least {}
    print(f"Viable Exit : {list(exits.keys())}")

    command = input("Where would you head for?(type in your option and q means exit):").strip().lower()

    if command == 'q':
        print("You've exited the dungeon.")
        break
    
    elif command == 'take':
        item = room_info.get("item")

        if item:
            inventory.append(item)
            print(f"You picked {item} successfully!")

            del dungeon_data[current_room_id]['item']
        else:
            print("It's totally empty here")

    elif command in exits:
        exit_data= exits[command]

        if isinstance(exit_data,str):
            current_room_id = exit_data
            print(f"You went towards {command}")
        elif isinstance(exit_data,dict):
            target_room = exit_data['target']
            is_locked = exit_data.get('locker',False) # default unlocked

            if is_locked:
                key_needed = exit_data.get('key_needed')

                if key_needed in inventory:
                    print("\n" + exit_data.get('unlock_msg', "Door Has Opened."))
                    current_room_id = target_room
                else:
                    print('\n' + exit_data.get('locked_msg',"Door's Locked!"))
            else:
                current_room_id = target_room
                print(f"Youwent towards{command}")

        if current_room_id == 'boss_room':
            print("\n" + "="*30)
            print("🐲 吼！！！(Red Dragon 把你烤熟了)")
            print("💀 GAME OVER")
            break

    else:
        print(f"{command} isn't leading you anywhere")



