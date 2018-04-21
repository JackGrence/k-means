# k-means algorithm implementation in C

## Getting Started

### Installing

#### For Linux:

Just `make`.

```
make
```

If you want debug with gdb, you can:

```
make debug
```

#### For Windows:

Please install `x86_64-w64-mingw32-gcc`,
You can use it to compile windows PE file on Linux:

```
make windows
```

## Usage

```
./kmeans FILE.jpg k
```

## Example

```
./kmeans test.jpg 9
```

Output file name is 9est.jpg

### Original picture

![](https://i.imgur.com/rCc0rAJ.png)

### Output picture

![](https://i.imgur.com/zRSniHA.png)
