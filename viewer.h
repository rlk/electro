#ifndef VIEWER_H
#define VIEWER_H

/*---------------------------------------------------------------------------*/

int viewer_point(int, int);
int viewer_click(int, int);

void viewer_init(void);
void viewer_draw(void);
void viewer_bill(void);

void viewer_get_pos(double[3]);
void viewer_get_vec(double[3]);

/*---------------------------------------------------------------------------*/

#endif
