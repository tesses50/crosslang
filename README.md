Tesses Cross Language
=====================

![CrossImage](logo.png)

## What is required to build this project
- [TessesFramework](https://onedev.site.tesses.net/tesses-framework)
- Jansson (but can be turned off but is strongly recommended otherwise many programs will not work)
- CMake
- SDL2 (but can be turned off)

## Use in docker (use my container)
```bash
    sudo docker pull -t onedev.site.tesses.net/crosslang/crosslang:latest
```

## Use in docker (build the container yourself)
```bash
    git clone https://onedev.site.tesses.net/crosslang
    cd crosslang
    sudo docker build -t crosslang:latest .
```

## To Install
- [Follow TessesFramework setup](https://onedev.site.tesses.net/tesses-framework)
- Follow the commands bellow

## Run these commands to install crosslang (or use binaries from [here](https://crosslang.tesseslanguage.com/download/))
```bash
git clone https://onedev.site.tesses.net/CrossLang
cd CrossLang
mkdir build 
cd build
cmake -S .. -B .
make
sudo make install
```

```
Ye are of God, little children, and have overcome them: because greater is he that is in you, than he that is in the world. (1 John 4:4)
```

