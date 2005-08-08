
--w = 2560
--h = 3200
w = 512
h = 640

tile = { }

host    = E.add_host("default", 0, 0, w, h)
tile[1] = E.add_tile(host, 0,     0, w, h / 2)
tile[2] = E.add_tile(host, 0, h / 2, w, h / 2)

E.set_tile_position(tile[1], -1.0, -0.625, -3.0, 2.0, 0.0, 0.0, 0.0, 1.25, 0.0)
E.set_tile_position(tile[2], -1.0, -0.625, -3.0, 2.0, 0.0, 0.0, 0.0, 1.25, 0.0)

E.set_tile_viewport(tile[1], 0, 0, w, h / 2)
E.set_tile_viewport(tile[2], 0, 0, w, h / 2)

E.set_tile_view_offset(tile[1], -0.05, 0, 0)
E.set_tile_view_offset(tile[2],  0.05, 0, 0)
E.set_tile_view_mirror(tile[2],  0, 0.707, 0.707, -1)

--E.set_host_flags(host,    E.host_flag_full,   true)

E.set_tile_flags(tile[2], E.tile_flag_flip_y, true)
