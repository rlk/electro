#ifndef STATUS_H
#define STATUS_H

/*---------------------------------------------------------------------------*/

void status_init(void);

void status_draw_camera(void);

/*---------------------------------------------------------------------------*/

void  status_set_viewport(int, int, int, int, int, int);

void  status_set_camera_pos(float, float, float);
void  status_set_camera_rot(float, float, float);

void  status_set_camera_dist(float);
void  status_set_camera_magn(float);
void  status_set_camera_zoom(float);

/*---------------------------------------------------------------------------*/

int   status_get_viewport_w(void);
int   status_get_viewport_h(void);

void  status_get_camera_pos(float *, float *, float *);
void  status_get_camera_rot(float *, float *, float *);

float status_get_camera_dist(void);
float status_get_camera_magn(void);
float status_get_camera_zoom(void);

/*---------------------------------------------------------------------------*/

#endif
