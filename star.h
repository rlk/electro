#ifndef STAR_H
#define STAR_H

/*---------------------------------------------------------------------------*/

#define STAR_TXT_RECLEN 451
#define STAR_BIN_RECLEN sizeof (struct star)

/*---------------------------------------------------------------------------*/

struct star
{
    GLubyte col[3];
    GLfloat pos[3];
    GLfloat mag;
};

/*---------------------------------------------------------------------------*/

int star_write_catalog(const char *);

int star_read_catalog_txt(const char *);
int star_read_catalog_bin(const char *);

void star_draw(void);

/*---------------------------------------------------------------------------*/

#endif
