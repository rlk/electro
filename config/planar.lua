
w = 2560
h = 1024

tile = { }

host    = E.add_host("default", 0, 0, w, h)
tile[1] = E.add_tile(host,     0, 0, w / 2, h)
tile[2] = E.add_tile(host, w / 2, 0, w / 2, h)

E.set_tile_position(tile[1], -0.5, -0.375, -2.0, 1.0, 0.0, 0.0, 0.0, 0.75, 0.0)
E.set_tile_position(tile[2], -0.5, -0.375, -2.0, 1.0, 0.0, 0.0, 0.0, 0.75, 0.0)

E.set_tile_viewport(tile[1], 0, 0, w / 2, h)
E.set_tile_viewport(tile[2], 0, 0, w / 2, h)

E.set_tile_view_offset(tile[1], -0.05, 0, 0)
E.set_tile_view_offset(tile[2],  0.05, 0, 0)

E.set_tile_flag(tile[2], E.tile_flag_flip_x, true)