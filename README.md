Tesses Cross Language
=====================

![CrossImage](crosslang.png)

## What is required to build this project
- [TessesFramework](https://onedev.site.tesses.net/tesses-framework) (if you turn CROSSLANG_FETCHCONTENT off otherwise it will be grabbed automaticly)
- CMake

## What libraries have I embeded from other people
- [HodwardHinnart's date]() (in folder src/HodwardHinnart_date, modified tz.cpp to point to correct header)
- [Sqlite]() (in folder src/sqlite also uses demo VFS (at least on Wii))
- [Sago's platform_folders]() (in folder src/sago)
- [subprocess](https://github.com/sheredom/subprocess.h) (as file src/runtime_methods/subprocess.h)
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
- Install [mbedtls](https://github.com/Mbed-TLS/mbedtls) (use sudo apt install libmbedtls-dev on debian) for TessesFramework
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
