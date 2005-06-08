-- Configure a simple default window with a line screen.

W =  1.0000
H =  0.7500
D =  2.0000
O =  0.2083

w = 1600
h = 1200

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_position(tile, -W / 2, -H / 2, -D, W, 0, 0, 0, H, 0)
E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_line_screen(tile, 271.945865, -7.88, 0.1, 0.0, 0.7777777)

E.set_host_flag(host, E.host_flag_framed, true)

------------------------------------------------------------------------------

function tile_test(b)
    E.set_tile_flag(tile, E.tile_flag_test, b)
end