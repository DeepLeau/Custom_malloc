#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

// void* easy_malloc(size_t size){
//     void* break_addr=sbrk(0);
//     if(sbrk(size)==(void *)-1){
//         return NULL;
//     }
//     return break_addr;
// }

#define multiple_of_four(x) (((((x)-1)/4)*4)+4)
#define INFO_BLOCK_SIZE 20
void *base=NULL;

struct memory_block {
size_t size;
struct memory_block *next;
struct memory_block *prev;
int custom_free;
void *ptr;
char data[1];
};
typedef struct memory_block *next_block;

next_block get_block(void *p){
    char *tmp;
    tmp = p;
    return (p = tmp -= INFO_BLOCK_SIZE );
}

int valid_addr(void *p){
    if (base){
        if (p>base && p<sbrk(0)){
            return (p==(get_block(p))->ptr);
        }
    }
    return (0);
}

next_block fusion(next_block b){
    if (b->next && b->next ->custom_free ){
        b->size += INFO_BLOCK_SIZE + b->next ->size;
        b->next = b->next ->next;
        if (b->next)
            b->next ->prev = b;
    }
    return (b);
}

next_block find_block (next_block *last , size_t size ){
    next_block b=base;
    while (b && !(b->custom_free && b->size >= size )) {
        *last = b;
        b = b->next;
    }
    return (b);
}

next_block extend_heap (next_block last , size_t s){
    next_block b;
    b = sbrk (0);
    if (sbrk(INFO_BLOCK_SIZE+s) == (void*)-1)
    //SBRK fails, on return NULL
        return (NULL );
    b->size = s;
    b->next = NULL;
    b->prev=last;
    b->ptr=b->data;
    if (last)
        last->next=b;
    b->custom_free=0;
    return (b);
}

void split_block (next_block b, size_t s){
    next_block new;
    new = (next_block)(b->data+s);
    new->size = b->size-s-INFO_BLOCK_SIZE ;
    new->next = b->next;
    new->prev = b;
    new->custom_free = 1;
    new->ptr = new->data;
    b->size = s;
    b->next = new;
    if (new->next)
        new->next->prev = new;
}

void copy_block (next_block src , next_block dst){
    int *sdata ,*ddata;
    size_t i;
    sdata = src ->ptr;
    ddata = dst ->ptr;
    for (i=0; i*4<src->size && i*4<dst->size; i++)
        ddata[i] = sdata[i];
}

void *custom_malloc(size_t size){
    next_block b,last;
    size_t s = multiple_of_four(size);
    if (base) {
        // Cherche un bloc libre de la bonne taille 
        last = base;
        b = find_block(&last,s);
        if(b){
            // Y a-t-il la place de store infos+au moins 4 bytes
            if ((b->size-s)>=(INFO_BLOCK_SIZE+4))
                split_block(b,s);
            b->custom_free =0;
        } else {
        //Pas de bloc -> augmente la HEAP
            b=extend_heap(last,s);
            if(!b)
                return(NULL);
        }
    } else {
        //Pas de bloc initialisé -> augmente la HEAP
        b=extend_heap(NULL,s);
        if(!b)
            return(NULL);
        base=b;
    }
    return(b->data);
}

void *custom_calloc(size_t number,size_t size){
    size_t *new;
    size_t s4,i;
    new=custom_malloc(number*size);
    if(new){
        s4=multiple_of_four(number*size);
        for(i=0;i<s4;i++)
            new[i]=0;
    }
    return new;
}

void custom_free(void *p){
    next_block b;
    if (valid_addr(p)){
        b = get_block (p);
        b->custom_free = 1;
        /* fusion with previous if possible */
        if(b->prev && b->prev ->custom_free)
            b = fusion(b->prev );
        /* then fusion with next */
        if (b->next)
            fusion(b);
        else{
            /* custom_free the end of the heap */
            if (b->prev)
                b->prev ->next = NULL;
            else
            /* No more block !*/
                base = NULL;
            brk(b);
        }
    }
}

void *custom_realloc(void *p, size_t size){
    size_t s;
    next_block b, new;
    void *newp;
    if (!p)
        return (malloc(size ));
        if ( valid_addr (p)){
        s = multiple_of_four(size);
        b = get_block (p);
        if (b->size >= s){
            if (b->size - s >= ( INFO_BLOCK_SIZE + 4))
                split_block (b,s);
        }
        else{
            // On essaye de fusionner avec le prochain
            if (b->next && b->next ->custom_free && (b->size + INFO_BLOCK_SIZE + b->next ->size) >= s)
                {
                fusion(b);
                if (b->size-s >= (INFO_BLOCK_SIZE + 4))
                    split_block(b,s);
                }
            else
            {
                newp = malloc(s);
                if (!newp)
                    return (NULL);
                new = get_block(newp );
                copy_block(b,new );
                custom_free(p);
                return (newp);
            }
        }
        return (p);
    }
    return (NULL );
}


int main(int argc,char *argv[]){
    // Test custom_malloc
    void *mem1 = custom_malloc(120);
    if (mem1 == NULL) {
        printf("custom_malloc a échoué.\n");
        return 1;
    }
    printf("L'adresse du pointeur alloué par custom_malloc est : %p\n", mem1);

    // Test custom_calloc
    size_t *mem2 = custom_calloc(5, sizeof(size_t));
    if (mem2 == NULL) {
        printf("custom_calloc a échoué.\n");
        return 1;
    }
    printf("L'adresse du pointeur alloué par custom_calloc est : %p\n", (void *)mem2);

    // Test custom_free
    void *mem3 = mem1;
    custom_free(mem1);
    if (mem1 == mem3) {
        printf("La libération d'espace a échoué.\n");
        return 1;
    }
    printf("custom_free a fonctionné : %p\n", mem1);

    // Test custom_realloc
    void *mem4 = custom_realloc(mem2, 10 * sizeof(size_t));
    if (mem4 == NULL) {
        printf("custom_realloc a échoué.\n");
        return 1;
    }
    printf("L'adresse du pointeur alloué par custom_realloc est : %p\n", mem4);

    // Libérer la mémoire allouée avec custom_realloc
    custom_free(mem4);

    return 0;
}