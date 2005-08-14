tile = { }

w = 800
h = 600
k = 0.95

w2 = w / 2
h2 = h / 2

host = E.add_host("default", 0, 0, w, h)

E.set_host_flags(host, E.host_flag_framed, true)

tile[0] = E.add_tile(host,  0, 0,  k * w2, k * h2)
tile[1] = E.add_tile(host, w2, 0,  k * w2, k * h2)
tile[2] = E.add_tile(host,  0, h2, k * w2, k * h2)
tile[3] = E.add_tile(host, w2, h2, k * w2, k * h2)

E.set_tile_viewport(tile[0],  0,  0, k * w2, k * h2)
E.set_tile_viewport(tile[1], w2,  0, k * w2, k * h2)
E.set_tile_viewport(tile[2],  0, h2, k * w2, k * h2)
E.set_tile_viewport(tile[3], w2, h2, k * w2, k * h2)

E.set_tile_position(tile[0], -1, -0.75, -1, k, 0, 0, 0, 0.75 * k, 0)
E.set_tile_position(tile[1],  0, -0.75, -1, k, 0, 0, 0, 0.75 * k, 0)
E.set_tile_position(tile[2], -1,  0,    -1, k, 0, 0, 0, 0.75 * k, 0)
E.set_tile_position(tile[3],  0,  0,    -1, k, 0, 0, 0, 0.75 * k, 0)
