
W =  1.0000
H =  0.7500
D =  1.5000
O =  0.2083

w = 1024
h =  768

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_position(tile, -W / 2, -H / 2, -D, W, 0.0, 0.0, 0.0, H, 0.0)
E.set_tile_viewport(tile, 0, 0, w, h)

E.set_host_flags(host, E.host_flag_full,   true)

------------------------------------------------------------------------------
