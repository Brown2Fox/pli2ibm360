### Description

```
.
├── ibm360vm - IBM360 Virtual Machine
│   ├── .idea
│   ├── CMakeLists.txt
│   ├── Makefile
│   └── ibm360vm.c
├── pli2asm - PL1 Translator to IBM360 asm
│   ├── .idea
│   ├── CMakeLists.txt
│   ├── Makefile
│   ├── pli2asm.l - Flex source code for PL1 tokenizer
│   └── pli2asm.y - Bison source code for PL1 parser
├── asm2obj - IBM360 asm Compiler to executable file
│   ├── .idea
│   ├── CMakeLists.txt
│   ├── Makefile
│   └── asm2obj.c
├── build.sh
├── CMakeLists.txt
├── examppl.pli
├── Makefile
├── README.md
├── run.sh
└── spis.mod
```

### Compilation

* *First way:* `$ make`
* *Second way:* `$ ./build.sh`
* *Third way:* use CLion projects for every target

### Execution

```
$ ./run.sh
```

### Dependencies

For Ubuntu:
1. Flex `$ sudo apt install flex`
2. Bison `$ sudo apt install bison`
3. ncurses `$ sudo apt install libncurses5-dev libncursesw5-dev`

For Windows (assuming you use MSYS2):
1. Flex `$ pacman -S flex`
2. Bison `$ pacman -S bison`
3. ncurses `$ pacman -S ncurses`

Also, you should change value of `MSYS_DIR` var in CMakeLists.txt (pli2asm project)