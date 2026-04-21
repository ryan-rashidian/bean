# bean

Program for counting characters, words, and lines from files or input. Supplied with a directory name, it will scan all the text files contained within it.

## Build Instructions

A `Makefile` is provided for building the program (using `gcc` with `-std=gnu99` compiler flag).

## Usage

- Count characters in a text file

```bash
bean -c example.txt
```

- Count characters and lines in a text file

```bash
bean -c -l example.txt
```

- Count all files in a directory with all options enabled by default

```bash
bean example/
```

- Enter input mode, and count characters (`Ctrl+D` to submit input)

```bash
bean -c
```

