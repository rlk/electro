#ifndef SERVER_H
#define SERVER_H

/*---------------------------------------------------------------------------*/

void server_send_draw(void);
void server_send_move(float, float, float);
void server_send_turn(float, float, float);
void server_send_zoom(float);
void server_send_dist(float);
void server_send_magn(float);

void server(int, int, char **);

/*---------------------------------------------------------------------------*/

#endif
