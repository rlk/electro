#ifndef VECTOR_H
#define VECTOR_H

/*---------------------------------------------------------------------------*/

typedef struct vector *vector_t;

/*---------------------------------------------------------------------------*/

vector_t vecnew(int, int);
void     vecdel(vector_t);
int      vecnum(vector_t);
int      vecadd(vector_t);
void    *vecget(vector_t, int);

/*---------------------------------------------------------------------------*/

#endif
