#ifndef SCRIPT_H
#define SCRIPT_H

/*---------------------------------------------------------------------------*/

int  script_init(void);
void script_free(void);

void script_file(const char *);

void script_click(int, int);
void script_point(int, int);
void script_keybd(int, int);

/*---------------------------------------------------------------------------*/

#endif
