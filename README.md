# go-socket-client [![Build Status](https://github.com/YanKawaYu/go-socket-client/workflows/Building/badge.svg?branch=main)](https://github.com/YanKawaYu/go-socket-client/actions?query=branch%3Amain)
Go-socket-client is the client library for the [go-socket](https://github.com/YanKawaYu/go-socket) server. It's written by C++, therefore it can be used on different platforms including Windows, MacOS, iOS, Android, etc.

## Getting started

### Install dependencies

If you want to build a library for iOS, then you need to run the `build_ios.py` on MacOS, since XCode is required during the building process.

If you want to build a library for Android, it's recommended to use [NDK r16b](https://dl.google.com/android/repository/android-ndk-r16b-linux-x86_64.zip).

If you want to build a library for MacOS, it's recommended to run the `build_osx.py` on MacOS, since the libtool only support -static option on MacOS.

It's recommended that the version of cmake should be newer than 3.6 and older than 3.19, since there could be problems if you use the latest version.

- **Ubuntu/Debian**

```bash
 $ sudo apt update && sudo apt install -y git python cmake unzip wget
 $ cd /opt && wget https://dl.google.com/android/repository/android-ndk-r16b-linux-x86_64.zip
 $ unzip android-ndk-r16b-linux-x86_64.zip -d .
 $ echo "export NDK_ROOT=/opt/android-ndk-r16b" >> ~/.bashrc && source ~/.bashrc
```

- **CentOS**

```bash
 $ sudo yum check-update && sudo yum install -y git python unzip wget

 $ cd /opt && wget https://github.com/Kitware/CMake/releases/download/v3.18.4/cmake-3.18.4-Linux-x86_64.tar.gz
 $ tar -zxvf cmake-3.18.4-Linux-x86_64.tar.gz
 $ mv cmake-3.18.4-Linux-x86_64 /usr/local/cmake
 $ echo 'export PATH="/usr/local/cmake/bin:$PATH"' >> ~/.bashrc && source ~/.bashrc

 $ cd /opt && wget https://dl.google.com/android/repository/android-ndk-r16b-linux-x86_64.zip
 $ unzip android-ndk-r16b-linux-x86_64.zip -d .
 $ echo "export NDK_ROOT=/opt/android-ndk-r16b" >> ~/.bashrc && source ~/.bashrc
```

The cmake in CentOS is too old to be used, therefore we download a newer cmake from official site manually.

- **MacOS**

```bash
 $ sudo brew update && sudo brew install git python cmake unzip wget
 $ cd /opt && wget https://dl.google.com/android/repository/android-ndk-r16b-linux-x86_64.zip
 $ unzip android-ndk-r16b-linux-x86_64.zip -d .
 $ echo "export NDK_ROOT=/opt/android-ndk-r16b" >> ~/.bashrc && source ~/.bashrc
```

Install XCode from AppStore.

### Get Go-socket-client

```bash
 $ git clone https://github.com/YanKawaYu/go-socket-client.git
```

### Building library

Cd into the `go-socket-client` directory, run the following commands to build a library on different platforms.

- **iOS**
```bash
 $ chmod u+x build_ios.py && ./build_ios.py
```
If the building process completed successfully, the result library `gotcp.framework` would be under the relative path `cmake_build/iOS/Darwin.out`.

- **Android**
```bash
 $ chmod u+x build_android.py && ./build_android.py
```
If the building process completed successfully, the result librarys `libs` would be under the relative path `cmake_build/Android_out`.

- **MacOS**
```bash
 $ chmod u+x build_osx.py && ./build_osx.py
```
If the building process completed successfully, the result library `gotcp.framework` would be under the relative path `cmake_build/OSX/Darwin.out`.

### Examples

There are examples for Android app and iOS app in `examples` directory. Make sure you change the ip address and port number to your own server's before you run the projects.

## Contributing

For now, I'm the only one that maintaining Go-socket-client. Any pull requests, suggestions or issues are appreciated!

## License

Go-socket is under the MIT license. See the [LICENSE](/LICENSE) file for details.

Some of the codes are modified from [Mars](https://github.com/Tencent/mars). Thanks to their contributions.
