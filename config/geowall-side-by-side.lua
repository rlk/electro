-- This Geowall configuration assumes a side-by-side dual-monitor desktop.

W =  5.1666  -- The Geowall in the AG Room is 5'2"
H =  4.1666  --                            by 4'2".
D = 10.0000  -- The ideal viewing position is 10' from the center.
O =  0.2083  -- The interocular distance is 2.5".

w = 2560
h = 1024

------------------------------------------------------------------------------

tile = { }

host    = E.add_host("default", 0, 0, w, h)
tile[1] = E.add_tile(host,     0, 0, w / 2, h)
tile[2] = E.add_tile(host, w / 2, 0, w / 2, h)

E.set_tile_position(tile[1], -W / 2, -H / 2, -D, W, 0.0, 0.0, 0.0, H, 0.0)
E.set_tile_position(tile[2], -W / 2, -H / 2, -D, W, 0.0, 0.0, 0.0, H, 0.0)

E.set_tile_viewport(tile[1], 0, 0, w / 2, h)
E.set_tile_viewport(tile[2], 0, 0, w / 2, h)

E.set_tile_view_offset(tile[1], -O / 2, 0, 0)
E.set_tile_view_offset(tile[2],  O / 2, 0, 0)

------------------------------------------------------------------------------
