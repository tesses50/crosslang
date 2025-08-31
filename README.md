Tesses Cross Language
=====================

![CrossImage](crosslang.png)

[WebSite](https://crosslang.tesseslanguage.com/)

## What is required to build this project
- [TessesFramework](https://onedev.site.tesses.net/tesses-framework) (if you turn CROSSLANG_FETCHCONTENT off otherwise it will be grabbed automaticly)
- CMake


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

## Run these commands to install crosslang (with plugin support)
```bash
git clone https://onedev.site.tesses.net/tesses-framework
cd tesses-framework
mkdir build 
cd build
cmake -S .. -B .
make
sudo make install
cd ../..
git clone https://onedev.site.tesses.net/CrossLang
cd CrossLang
mkdir build 
cd build
cmake -S .. -B . -DCROSSLANG_FETCHCONTENT=OFF
make
sudo make install
```

## Build with shared libs only (self contained dependencies)
```bash
cmake -S ../.. -B . -DTESSESFRAMEWORK_ENABLE_STATIC=OFF -DTESSESFRAMEWORK_ENABLE_SHARED=ON -DTESSESFRAMEWORK_FETCHCONTENT=ON
make -j`nproc`
sudo make install
```

```bash
git clone https://onedev.site.tesses.net/CrossLang
cd CrossLang
mkdir build 
cd build
cmake -S .. -B .
make
sudo make install
```

## To Create the binaries and ISO (You need linux for this)
```bash
sudo apt install xorriso wget curl mingw-w64 nsis cmake tar zip
mkdir dvdwork
cp build-dvd.tcross dvdwork/
cd dvdwork
crossint build-dvd.tcross
bash script.sh # the iso and tarballs will be in Working/Output
```

```
Ye are of God, little children, and have overcome them: because greater is he that is in you, than he that is in the world. (1 John 4:4)
```
