host = { }
tile = { }

a = 4 / 3

w = 1600
h = 1200

x = { -2, -1, 0, 1 }
y = { -2.5 / a, -1.5 / a, -0.5 / a, 0.5 / a, 1.5 / a }
z   = -2

L = { -3200, -1600, 0, 1600 }
B = { -3000, -1800, -600, 600, 1800 }

R = 0.92
U = 0.92 / a

host[0]  = E.add_host("nico",      0, 0, 800, 600)
host[1]  = E.add_host("nico1-10",  0, 0, w * 2, h)
host[2]  = E.add_host("nico2-10",  0, 0, w * 2, h)
host[3]  = E.add_host("nico3-10",  0, 0, w * 2, h)
host[4]  = E.add_host("nico4-10",  0, 0, w * 2, h)
host[5]  = E.add_host("nico5-10",  0, 0, w * 2, h)
host[6]  = E.add_host("nico6-10",  0, 0, w * 2, h)
host[7]  = E.add_host("nico7-10",  0, 0, w * 2, h)
host[8]  = E.add_host("nico8-10",  0, 0, w * 2, h)
host[9]  = E.add_host("nico9-10",  0, 0, w * 2, h)
host[10] = E.add_host("nico10-10", 0, 0, w * 2, h)

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
tile[14] = E.add_tile(host[7],  w, 0, w, h)
tile[15] = E.add_tile(host[8],  0, 0, w, h)
tile[16] = E.add_tile(host[8],  w, 0, w, h)
tile[17] = E.add_tile(host[9],  0, 0, w, h)
tile[18] = E.add_tile(host[9],  w, 0, w, h)
tile[19] = E.add_tile(host[10], 0, 0, w, h)
tile[20] = E.add_tile(host[10], w, 0, w, h)

E.set_tile_viewport(tile[0],  L[1], B[1], w * 4, h * 5)
E.set_tile_viewport(tile[1],  L[3], B[5], w, h)
E.set_tile_viewport(tile[2],  L[4], B[5], w, h)
E.set_tile_viewport(tile[3],  L[3], B[4], w, h)
E.set_tile_viewport(tile[4],  L[4], B[4], w, h)
E.set_tile_viewport(tile[5],  L[3], B[3], w, h)
E.set_tile_viewport(tile[6],  L[4], B[3], w, h)
E.set_tile_viewport(tile[7],  L[3], B[2], w, h)
E.set_tile_viewport(tile[8],  L[4], B[2], w, h)
E.set_tile_viewport(tile[9],  L[3], B[1], w, h)
E.set_tile_viewport(tile[10], L[4], B[1], w, h)
E.set_tile_viewport(tile[11], L[1], B[5], w, h)
E.set_tile_viewport(tile[12], L[2], B[5], w, h)
E.set_tile_viewport(tile[13], L[1], B[4], w, h)
E.set_tile_viewport(tile[14], L[2], B[4], w, h)
E.set_tile_viewport(tile[15], L[1], B[3], w, h)
E.set_tile_viewport(tile[16], L[2], B[3], w, h)
E.set_tile_viewport(tile[17], L[1], B[2], w, h)
E.set_tile_viewport(tile[18], L[2], B[2], w, h)
E.set_tile_viewport(tile[19], L[1], B[1], w, h)
E.set_tile_viewport(tile[20], L[2], B[1], w, h)

E.set_tile_position(tile[0],  x[1], x[1], z, R * 4, 0, 0, 0, U * 5, 0)
E.set_tile_position(tile[1],  x[3], y[5], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[2],  x[4], y[5], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[3],  x[3], y[4], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[4],  x[4], y[4], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[5],  x[3], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[6],  x[4], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[7],  x[3], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[8],  x[4], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[9],  x[3], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[10], x[4], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[11], x[1], y[5], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[12], x[2], y[5], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[13], x[1], y[4], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[14], x[2], y[4], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[15], x[1], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[16], x[2], y[3], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[17], x[1], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[18], x[2], y[2], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[19], x[1], y[1], z, R, 0, 0, 0, U, 0)
E.set_tile_position(tile[20], x[2], y[1], z, R, 0, 0, 0, U, 0)

-------------------------------------------------------------------------------
