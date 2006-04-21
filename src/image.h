/*    Copyright (C) 2005 Robert Kooima                                       */
/*                                                                           */
/*    ELECTRO is free software;  you can redistribute it and/or modify it    */
/*    under the terms of the  GNU General Public License  as published by    */
/*    the  Free Software Foundation;  either version 2 of the License, or    */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful, but    */
/*    WITHOUT  ANY  WARRANTY;  without   even  the  implied  warranty  of    */
/*    MERCHANTABILITY or  FITNESS FOR A PARTICULAR PURPOSE.   See the GNU    */
/*    General Public License for more details.                               */

#ifndef IMAGE_H
#define IMAGE_H

/*---------------------------------------------------------------------------*/

int startup_image(void);

/*---------------------------------------------------------------------------*/

int  send_create_image_udp(int);
int  send_create_image_nil(int, int);
int  send_create_image_map(const char *);
int  send_create_image_ani(const char *, int, int, int, int, int, int, int);
int  send_create_image_env(const char *, const char *,
                           const char *, const char *,
                           const char *, const char *);

int  dupe_create_image(int);
void recv_create_image(void);

int  send_create_video(int);
void send_delete_image(int);
void recv_delete_image(void);

void send_set_image_pixels(int, void *, int, int, int, int, int, int);
void recv_set_image_pixels(void);

/*---------------------------------------------------------------------------*/

GLenum get_image_target(int);
GLuint get_image_buffer(int);

void get_image_c(int, int, int, unsigned char[4]);
int  get_image_w(int);
int  get_image_h(int);

/*---------------------------------------------------------------------------*/

void  init_image(int);
void  fini_image(int);
void  draw_image(int);

void  nuke_images(void);
void  init_images(void);
void  fini_images(void);
void  step_images(void);

/*---------------------------------------------------------------------------*/

#endif
