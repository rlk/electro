-- Configure a simple default window with a line screen.

W =  1.3333333
H =  1.0000000
D =  4.0000000
O =  0.2083333

w = 1600
h = 1200

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_position(tile, -W / 2, -H / 2, -D, W, 0, 0, 0, H, 0)
E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_line_screen(tile, 271.945865, -7.88, 0.037, 0.0037, 0.777777)
--E.set_tile_line_screen(tile, 12, -7.88, 0.037, 0.0037, 0.777777)

--E.set_host_flag(host, E.host_flag_framed, true)
--E.set_host_flag(host, E.host_flag_full,   true)

------------------------------------------------------------------------------

function tile_test(b)
    E.set_tile_flag(tile, E.tile_flag_test, b)
end

E.set_background(1, 1, 1)
