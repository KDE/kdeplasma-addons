import fileinput

for line in fileinput.input():
    if line.startswith("LayoutNames=thumbnails"):
        print("LayoutNames=org.kde.breeze.desktop")
