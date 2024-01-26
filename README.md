# Custom_malloc

## get_block method :
This method returns a pointer to the memory block according to the address passed.

## valid_addr method :
Checks if an address is valid and within the allocated memory area.

## fusion method :
Merges two adjacent memory blocks if both are free.

## find_block method :
Finds a free memory block of the specified size.

## extend_heap method :
Extends the allocated memory area if necessary.

## split_block method :
Divides a block of memory into two parts (used when releasing part of a block).

## copy_block method :
Copies the contents of one block to another.
