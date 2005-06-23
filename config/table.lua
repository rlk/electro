host = { }
tile = { }
L = { }
B = { }

a = 4 / 3

w = 1280
h = 1024

ppf = 12 * 1280 / 14

dx = 3.520
dy = 1.708

x = { 0.375, 1.645, 2.937, 4.208, 5.489 }
y = { 0.229, 1.270, 2.312 }
z = 4

x[1] = x[1] - dx
x[2] = x[2] - dx
x[3] = x[3] - dx
x[4] = x[4] - dx
x[5] = x[5] - dx

y[1] = y[1] - dy
y[2] = y[2] - dy
y[3] = y[3] - dy

L[1] = x[1] * ppf
L[2] = x[2] * ppf
L[3] = x[3] * ppf
L[4] = x[4] * ppf
L[5] = x[5] * ppf

B[1] = y[1] * ppf
B[2] = y[2] * ppf
B[3] = y[3] * ppf

R = w / ppf
U = h / ppf

host[0]  = E.add_host("yorda",      0, 0, 800, 600)
host[1]  = E.add_host("yorda1-10",  0, 0, w * 2, h)
host[2]  = E.add_host("yorda2-10",  0, 0, w * 2, h)
host[3]  = E.add_host("yorda3-10",  0, 0, w * 2, h)
host[4]  = E.add_host("yorda4-10",  0, 0, w * 2, h)
host[5]  = E.add_host("yorda5-10",  0, 0, w * 2, h)
host[6]  = E.add_host("yorda6-10",  0, 0, w * 2, h)
host[7]  = E.add_host("yorda7-10",  0, 0, w,     h)
host[8]  = E.add_host("yorda8-10",  0, 0, w,     h)
host[9]  = E.add_host("yorda9-10",  0, 0, w,     h)

tile[0]  = E.add_tile(host[0],  0, 0, w, h)
tile[1]  = E.add_tile(host[1],  0, 0, w, h)
tile[2]  = E.add_tile(host[1],  w, 0, w, h)
tile[3]  = E.add_tile(host[2],  0, 0, w, h)
tile[4]  = E.add_tile(host[2],  w, 0, w, h)
tile[5]  = E.add_tile(host[3],  0, 0, w, h)
tile[6]  = E.add_tile(host[3],  w, 0, w, h)
tile[7]  = E.add_tile(host[4],  0, 0, w, h)
tile[8]  = E.add_tile(host[4],  w, 0, w, h)
tile[9]  = E.add_tile(host[5],  0, 0, w, h)
tile[10] = E.add_tile(host[5],  w, 0, w, h)
tile[11] = E.add_tile(host[6],  0, 0, w, h)
tile[12] = E.add_tile(host[6],  w, 0, w, h)
tile[13] = E.add_tile(host[7],  0, 0, w, h)
tile[14] = E.add_tile(host[8],  0, 0, w, h)
tile[15] = E.add_tile(host[9],  0, 0, w, h)

E.set_tile_viewport(tile[0],  L[1], B[1], w * 5, h * 3)
E.set_tile_viewport(tile[1],  L[1], B[3], w, h)
E.set_tile_viewport(tile[2],  L[2], B[3], w, h)
E.set_tile_viewport(tile[3],  L[1], B[2], w, h)
E.set_tile_viewport(tile[4],  L[2], B[2], w, h)
E.set_tile_viewport(tile[5],  L[1], B[1], w, h)
E.set_tile_viewport(tile[6],  L[2], B[1], w, h)
E.set_tile_viewport(tile[7],  L[3], B[3], w, h)
E.set_tile_viewport(tile[8],  L[4], B[3], w, h)
E.set_tile_viewport(tile[9],  L[3], B[2], w, h)
E.set_tile_viewport(tile[10], L[4], B[2], w, h)
E.set_tile_viewport(tile[11], L[3], B[1], w, h)
E.set_tile_viewport(tile[12], L[4], B[1], w, h)
E.set_tile_viewport(tile[13], L[5], B[3], w, h)
E.set_tile_viewport(tile[14], L[5], B[2], w, h)
E.set_tile_viewport(tile[15], L[5], B[1], w, h)

E.set_tile_position(tile[0],  x[1], y[1], z, R * 5, 0, 0, 0, U * 3, 0)
E.set_tile_position(tile[1],  x[1], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[2],  x[2], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[3],  x[1], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[4],  x[2], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[5],  x[1], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[6],  x[2], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[7],  x[3], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[8],  x[4], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[9],  x[3], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[10], x[4], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[11], x[3], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[12], x[4], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[13], x[5], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[14], x[5], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[15], x[5], y[1], z, R, 0, 0, 0, U, 0)

E.set_host_flag(host[0], E.host_flag_framed, true)

-------------------------------------------------------------------------------
