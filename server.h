#ifndef SERVER_H
#define SERVER_H

/*---------------------------------------------------------------------------*/

void server_send_draw(void);
void server_send_move(void);
void server_send_turn(void);
void server_send_zoom(void);
void server_send_dist(void);
void server_send_magn(void);

void server(int, int, char **);

/*---------------------------------------------------------------------------*/

#endif
