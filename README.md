Tesses Cross Language
=====================

![CrossImage](crosslang.png)

[WebSite](https://crosslang.tesseslanguage.com/)

## What is required to build this project
- [TessesFramework](https://git.tesses.org/tesses50/tessesframework) (if you turn CROSSLANG_FETCHCONTENT off otherwise it will be grabbed automaticly)
- CMake


## Use in docker (use my container)
```bash
    sudo docker pull -t git.tesses.org/tesses50/crosslang:latest
```

## Use in docker (build the container yourself)
```bash
    git clone https://git.tesses.org/tesses50/crosslang
    cd crosslang
    sudo docker build -t crosslang:latest .
```

## To Install
- Install [mbedtls](https://github.com/Mbed-TLS/mbedtls) (use sudo apt install libmbedtls-dev on debian) for TessesFramework (no longer works on debian, needs -DTESSESFRAMEWORK_FETCHCONTENT=ON for debian)
- Follow the commands bellow

## Run these commands to install crosslang (with plugin support)
```bash
git clone https://git.tesses.org/tesses50/tessesframework
cd tessesframework
mkdir build 
cd build
cmake -S .. -B . -DTESSESFRAMEWORK_FETCHCONTENT=OFF
make
sudo make install
cd ../..
git clone https://git.tesses.org/tesses50/crosslang
cd crosslang
mkdir build 
cd build
cmake -S .. -B . -DCROSSLANG_FETCHCONTENT=OFF
make
sudo make install
```

## Build with shared libs only (self contained dependencies)
```bash
cmake -S ../.. -B .
make -j`nproc`
sudo make install
```

```
Ye are of God, little children, and have overcome them: because greater is he that is in you, than he that is in the world. (1 John 4:4)
```
