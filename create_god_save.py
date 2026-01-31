#!/usr/bin/env python3
import os

LEVELS_DIR = "levels"
SAVE_FILE = "save.txt"

def main():
    if not os.path.isdir(LEVELS_DIR):
        print(f"Error: '{LEVELS_DIR}' directory not found")
        return

    levels = []

    for filename in os.listdir(LEVELS_DIR):
        if filename.endswith(".txt"):
            path = os.path.join(LEVELS_DIR, filename)
            levels.append(path)

    levels.sort()

    with open(SAVE_FILE, "w", encoding="utf-8") as f:
        for lvl in levels:
            f.write(lvl + "\n")

    print(f"God save created: {SAVE_FILE}")
    print(f"{len(levels)} levels unlocked.")

if __name__ == "__main__":
    main()
