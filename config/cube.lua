
tile = { }

-------------------------------------------------------------------------------
-- This config renders the scene as the sides of a cubic environment map.
-- We assume a 4x3 aspect ratio to ensure each side is square.

w = 1600
h = 1200
s = w / 4

host = E.add_host("default", 0, 0, w, h)

E.set_host_flags(host, E.host_flag_framed, true)

tile[0] = E.add_tile(host, 0 * s, 1 * s, s, s)
tile[1] = E.add_tile(host, 2 * s, 1 * s, s, s)
tile[2] = E.add_tile(host, 1 * s, 0 * s, s, s)
tile[3] = E.add_tile(host, 1 * s, 2 * s, s, s)
tile[4] = E.add_tile(host, 1 * s, 1 * s, s, s)
tile[5] = E.add_tile(host, 3 * s, 1 * s, s, s)

E.set_tile_viewport(tile[0], 0, 0, s, s)
E.set_tile_viewport(tile[1], 0, 0, s, s)
E.set_tile_viewport(tile[2], 0, 0, s, s)
E.set_tile_viewport(tile[3], 0, 0, s, s)
E.set_tile_viewport(tile[4], 0, 0, s, s)
E.set_tile_viewport(tile[5], 0, 0, s, s)

E.set_tile_position(tile[0], -1, -1,  1,  0,  0, -2,  0,  2,  0)
E.set_tile_position(tile[1],  1, -1, -1,  0,  0,  2,  0,  2,  0)
E.set_tile_position(tile[2], -1, -1,  1,  2,  0,  0,  0,  0, -2)
E.set_tile_position(tile[3], -1,  1, -1,  2,  0,  0,  0,  0,  2)
E.set_tile_position(tile[4], -1, -1, -1,  2,  0,  0,  0,  2,  0)
E.set_tile_position(tile[5],  1, -1,  1, -2,  0,  0,  0,  2,  0)

