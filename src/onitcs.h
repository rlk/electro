// OPENNI TRACKER Copyright (C) 2011 Robert Kooima
//
// This program is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.

#ifndef ONITCS_H
#define ONITCS_H

//------------------------------------------------------------------------------

#define ONIT_SERVICE "25670"

struct onit_point
{
    float confidence;
    float world_p[3];
    float image_p[2];
};

typedef struct onit_point  onit_point;
typedef struct onit_server onit_server;
typedef struct onit_client onit_client;

//------------------------------------------------------------------------------

onit_server *onit_server_init(const char *);
void         onit_server_fini(onit_server *);
void         onit_server_poll(onit_server *);
void         onit_server_send(onit_server *, void *, size_t);

//------------------------------------------------------------------------------

onit_client *onit_client_init(const char *, const char *);
void         onit_client_fini(onit_client *);
size_t       onit_client_recv(onit_client *, void *, size_t);

//------------------------------------------------------------------------------

#endif
