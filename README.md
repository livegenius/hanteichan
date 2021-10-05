# Hantei-chan #
Hantei-chan is a HA6/Hantei6 GUI editor for French bread games. MBAACC support has been dropped.
The last release to support it was 1.1.1. This program only for windows. 

![screenshot](https://user-images.githubusercontent.com/39018575/119175726-311f4580-ba38-11eb-83dd-2d7d57c17f02.png)

-----------------------
## Hotkeys and stuff ##

Left click and drag in the display to scroll the view.
Drag right click to draw a box.
You can switch frames/patterns with the arrow keys.
You can switch the currently selected type of box with Z and X.
Don't try to press these hotkeys while you're drawing a box. You'll just make a mess.
You can view the all the shortcuts by going into Help->Shortcuts

-----------------------
## How to build ##
Building is extremely simple, and it works with both MSVC and Mingw-w64.
Clone the git, init the submodules and run cmake.
Assuming you're using ninja:

```
git clone --recursive [insert repo link here] hanteichan
cd hanteichan
mkdir build
cmake -S. -Bbuild -GNinja -DCMAKE_BUILD_TYPE=Release
cmake --build build
build\hanteichan.exe
```

And that's it!
