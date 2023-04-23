import sys

slangc_path = sys.argv[1]
with open("slangc_path.txt", "w") as f:
    f.write(slangc_path)
