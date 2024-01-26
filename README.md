# Custom_malloc

## Required methods for malloc and calloc :

### get_block method :
This method returns a pointer to the memory block according to the address passed.

### valid_addr method :
Checks if an address is valid and within the allocated memory area.

### fusion method :
Merges two adjacent memory blocks if both are free.

### find_block method :
Finds a free memory block of the specified size.

### extend_heap method :
Extends the allocated memory area if necessary.

### split_block method :
Divides a block of memory into two parts (used when releasing part of a block).

### copy_block method :
Copies the contents of one block to another.

## Calloc, malloc, free and realloc methods :
The purpose of these functions is to allocate memory on the heap of a program. It's working using brk(2) and sbrk(2) system calls and the previous methods described above.

## How to run the code :
First we need to create the object file :
``` gcc -o malloc malloc.c ```
Then execute it :
``` ./malloc ```

