/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


#define min(a,b) a<b?a:b
#define max(a,b) a<b?b:a

#define ARR_SIZE 80
size_t  *sz(void *p){return (size_t *)p;}
void  *rear(void *p){return (char *)p+(*sz(p)&-2);}
void *front(void *p){return (char *)p-(*sz(p-SIZE_T_SIZE)&-2);}
size_t  *bt(void *p){return (size_t *)((char *)rear(p)-SIZE_T_SIZE);}
void **next(void *p){return (void *)((char *)p+SIZE_T_SIZE);}
void **prev(void *p){return (void *)((char *)p+SIZE_T_SIZE*2);}

void *flag,*end,*root;
void **arr(int i){return (void *)((char *)root+SIZE_T_SIZE*i);}
int sz2idx(size_t x){return min(x/SIZE_T_SIZE,ARR_SIZE)-1;}

void del(void *p){
    int i=sz2idx(*sz(p));
    void *pv=*prev(p),*nx=*next(p);
    if(pv==arr(i))*arr(i)=nx;
    else *next(pv)=nx;
    if(nx!=NULL)*prev(nx)=pv;
}
void add(void *p){
    int i=sz2idx(*sz(p));
    void *t=*arr(i);
    *next(p)=t;
    if(t!=NULL)*prev(t)=p;
    *prev(p)=arr(i);
    *arr(i)=p;
}
/*
 * mm_init - initialize the malloc package.
 */

int mm_init(void)
{
    int i;
    root=mem_sbrk(SIZE_T_SIZE*ARR_SIZE);
    end=mem_heap_hi();
    for(i=0;i<ARR_SIZE;++i)*arr(i)=NULL;
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if(size==0)return NULL;
		size=max(size,SIZE_T_SIZE*2);
    int i,x,newsize = ALIGN(size + SIZE_T_SIZE*2);
    void *p,*r;
    x=sz2idx(newsize);
    for(i=x;i<ARR_SIZE;++i){
       for(p=*arr(i);p!=NULL;p=*next(p))if(*sz(p)>=newsize)break;
			 if(p!=NULL)break;
		}
    if(i>=ARR_SIZE){
				p=(void *)((char *)mem_heap_hi()-7);
				if(p>end&&!(*sz(p)&1)){
						p=(void *)((char *)p-*sz(p)+SIZE_T_SIZE);
						del(p);
						newsize-=*sz(p);
						mem_sbrk(newsize);
						*bt(p)=0;
						*sz(p)+=newsize+1;
				}
				else{
						p=mem_sbrk(newsize);
						*sz(p)=newsize|1;
				}
		}
    else{
        del(p);
        if(newsize+SIZE_T_SIZE*4<=*sz(p)){
            r=p+newsize;
            *sz(r)=*sz(p)-newsize;
            *bt(r)=*sz(r);
            add(r);
            *sz(p)=newsize|1;
        }
				else *sz(p)|=1;
    }

    if (p == (void *)-1)return NULL;

		printf("%p %p %p %d\n",p,sz(p),bt(p),*sz(p));
		*bt(p)=*sz(p);
    return (void *)((char *)p + SIZE_T_SIZE);
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    void *f=NULL,*r,*lo,*hi;
    size_t s;
    int F,R;
    lo=end;
    hi=mem_heap_hi();
    ptr=(void *)((char *)ptr-SIZE_T_SIZE);
		if(ptr>lo)f=front(ptr);
    r=rear(ptr);
    F=(ptr==lo)||(*sz(f)&1);
    R=(r>hi)||(*sz(r)&1);
    if(F&&R){
        *bt(ptr)^=1;
        *sz(ptr)^=1;
        add(ptr);
    }
    else if(F&&!R){
        del(r);
        s=*sz(ptr)+*sz(r)-1;
        *bt(ptr)=0;
        *sz(ptr)=*bt(r)=s;
        *sz(r)=0;
        add(ptr);
    }
    else if(!F&&R){
        del(f);
        s=*sz(f)+*sz(ptr)-1;
        *bt(f)=0;
        *sz(f)=*bt(ptr)=s;
        *sz(ptr)=0;
        add(f);
		}
    else{
        del(f);
        del(r);
        s=*sz(f)+*sz(ptr)+*sz(r)-1;
        *bt(f)=*bt(ptr)=0;
        *sz(f)=*bt(r)=s;
        *sz(ptr)=*sz(r)=0;
        add(f);
		}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    return NULL;
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
