#ifndef STAR_H
#define STAR_H

/*---------------------------------------------------------------------------*/

#define STAR_TXT_RECLEN 451
#define STAR_BIN_RECLEN  20

/*---------------------------------------------------------------------------*/

struct star
{
    float         position[3];
    float         magnitude;
    unsigned char color[3];
};

/*---------------------------------------------------------------------------*/

int star_read_catalog_txt(const char *);
int star_read_catalog_bin(const char *);

void star_draw(void);

/*---------------------------------------------------------------------------*/

#endif
