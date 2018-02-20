### Description

```
.
├── ibm360vm - IBM360 Virtual Machine
│   ├── Makefile
│   └── ibm360vm.c
├── pli2asm - PL1 Translator to IBM360 asm
│   ├── Makefile
│   ├── pli2asm.l - Flex source code for PL1 tokenizer
│   └── pli2asm.y - Bison source code for PL1 parser
├── asm2obj - IBM360 asm Compiler to executable file
│   ├── Makefile
│   └── asm2obj.c
├── examppl.pli
├── Makefile
├── README.md
├── run.sh
└── spis.mod
```

### Compilation

```
$ make
```

### Execution

```
$ ./run.sh
```

