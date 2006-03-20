linescreen_file = "/DEMO/varrier/varrier_configs/linescreen_setup.txt"
read_linescreen(linescreen_file, "screen1")

W =  2.0000
H =  1.1250
D =  2.0000
O =  0.2083

w = 2560
h = 1600

p = {
    { -1.0521, 3.1354, -1.3614 },
    { -1.0521, 4.4531, -1.3614 },
    {  1.0521, 3.1354, -1.3614 },
}

offset = { 0, (p[2][2] + p[1][2]) / 2, 2.0 + p[1][3] }

------------------------------------------------------------------------------

host = E.add_host("default", 0, 1200, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_position(tile, p[1][1], p[1][2], p[1][3],
                    p[3][1] - p[1][1], p[3][2] - p[1][2], p[3][3] - p[1][3],
                    p[2][1] - p[1][1], p[2][2] - p[1][2], p[2][3] - p[1][3])
E.set_tile_line_screen(tile, linescreen.p, linescreen.a,
                             linescreen.t, linescreen.s, linescreen.c)

E.set_host_flags(host, E.host_flag_framed, false)

-- Uncomment this line when head tracking is disabled.
--E.set_tile_view_offset(tile, 0, 4, 0)
--E.set_tile_view_offset(tile, 0, 0, -1)
