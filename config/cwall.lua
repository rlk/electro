-- This CWall configuration assumes a side-by-side dual-monitor desktop.

W = 10.0000
H =  8.0000
D = 10.0000  -- The ideal viewing position is 10' from the center.
O =  0.2083  -- The interocular distance is 2.5".

w = 2048
h =  768

stereo_mode = E.stereo_mode_tile

------------------------------------------------------------------------------

tile = { }

host    = E.add_host("default", 0, 0, w, h)
tile[1] = E.add_tile(host,     0, 0, w / 2, h)
tile[2] = E.add_tile(host, w / 2, 0, w / 2, h)

E.set_tile_position(tile[1], -W / 2, 0, -D, W, 0.0, 0.0, 0.0, H, 0.0)
E.set_tile_position(tile[2], -W / 2, 0, -D, W, 0.0, 0.0, 0.0, H, 0.0)

E.set_tile_viewport(tile[1], 0, 0, w / 2, h)
E.set_tile_viewport(tile[2], 0, 0, w / 2, h)

--E.set_host_flags(host, E.host_flag_full, true)

--E.set_tile_flags(tile[1], E.tile_flag_left_eye, true)
--E.set_tile_flags(tile[2], E.tile_flag_right_eye, true)

E.set_tile_flags(tile[1], E.tile_flag_right_eye, true)
E.set_tile_flags(tile[2], E.tile_flag_left_eye, true)

------------------------------------------------------------------------------

