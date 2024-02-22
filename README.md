# Slangc

LLVM-based compiler for Slang - educational programming language.

https://slangdocs.readthedocs.io/ru/latest/

## Building
Clone the repository
```shell
git clone https://github.com/f0xeri/SlangCompiler
```
Install LLVM 18 and libgc-dev
```shell
sudo wget https://apt.llvm.org/llvm.sh &&
sudo bash ./llvm.sh 18 all &&
sudo apt-get install libgc-dev -y
```

Build the project
```shell
cd SlangCompiler
```
```shell
cmake -B build -DCMAKE_BUILD_TYPE=Release
```
```shell
cmake --build build --config Release
```
Run tests
```shell
cd tests &&
python -m setSlangcPath "../build/slangc" && python -m unittest runTests.py
```