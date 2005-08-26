-- This Geowall configuration assumes quad-buffered stereo capability.

W =  5.1666  -- The Geowall in the AG Room is 5'2"
H =  4.1666  --                            by 4'2".
D = 10.0000  -- The ideal viewing position is 10' from the center.
O =  0.2083  -- The interocular distance is 2.5".

w = 1024
h =  768

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_position(tile, -W / 2, -H / 2, -D, W, 0.0, 0.0, 0.0, H, 0.0)
E.set_tile_viewport(tile, 0, 0, w, h)

E.set_host_flags(host, E.host_flag_stereo, true)
E.set_host_flags(host, E.host_flag_full,   true)

------------------------------------------------------------------------------
