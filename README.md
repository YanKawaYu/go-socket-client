# go-socket-client
Go-socket-client is the client library for the [go-socket](https://github.com/YanKawaYu/go-socket) server. It's written by C++, therefore it can be used on different platforms including Windows, MacOS, iOS, Android, etc.

## Getting started

### Setup

- **Clone from go-socket-client repository:**

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

## Contributing

For now, I'm the only one that maintaining Go-socket-client. Any pull requests, suggestions or issues are appreciated!

## License

Go-socket is under the MIT license. See the [LICENSE](/LICENSE) file for details.

Some of the codes are modified from [Mars](https://github.com/Tencent/mars). Thanks to their contributions.
