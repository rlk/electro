#ifndef VIEWER_H
#define VIEWER_H

/*---------------------------------------------------------------------------*/

void viewer_init(void);
void viewer_draw(void);
void viewer_bill(void);

int viewer_point(int, int);
int viewer_click(int, int);
int viewer_keybd(int, int);
int viewer_event(int);

void viewer_get_pos(double[3]);
void viewer_get_vec(double[3]);
void viewer_get_mag(double[1]);

/*---------------------------------------------------------------------------*/

#endif
