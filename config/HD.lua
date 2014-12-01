
W = 5.81 -- Display width in feet
H = 3.27 -- Display height in feet
D = 6.00 -- Display view distance in feet

w = 1920 -- Display horizontal resolution
h = 1080 -- Display vertical resolution

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_position(tile, -W / 2, -H / 2, -D, W, 0.0, 0.0, 0.0, H, 0.0)
E.set_tile_viewport(tile, 0, 0, w, h)

E.set_host_flags(host, E.host_flag_full, true)

------------------------------------------------------------------------------

