### Description

```
.
├── ibm360vm - IBM360 Virtual Machine
│   ├── CMakeLists.txt
│   └── ibm360vm.c
├── pli2asm - PL1 Translator to IBM360 asm
│   ├── CMakeLists.txt
│   ├── pli2asm.l - Flex source code for PL1 tokenizer
│   └── pli2asm.y - Bison source code for PL1 parser
├── pli2asm_v1 - Original PL1 Translator to IBM360 asm
│   ├── CMakeLists.txt
│   └── pli2asm_v1.c
├── asm2obj - IBM360 asm Compiler to executable file
│   ├── CMakeLists.txt
│   └── asm2obj.c
├── build.sh - for build
├── CMakeLists.txt - for build
├── run.sh - for run
├── Runfile - for run
├── README.md
└── examppl.pli 
```

### Compilation

* *First way:* `$ ./build.sh <target>` where target is a name of a project
* *Second way:* use CLion projects for every target

### Execution

```
$ ./run.sh [o1|o2|o3]

o1 - to pli2asm_v1 translate
o2 - to asm2obj translate
o3 - to run vm
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

Also, you should change value of `MSYS_DIR` var in Windows Environments Variables