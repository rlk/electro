
-------------------------------------------------------------------------------

mullion_w = 128
mullion_h = 128

w  = 1600
h  = 1200
dx = w + mullion_w
dy = h + mullion_h

total_w  =  dx * 4
total_h  =  dy * 4
total_x  = -total_w / 2
total_y  = -total_h / 2

E.add_tile("nico5-10",    0, 0, total_x,          total_y,          w, h)
E.add_tile("nico6-10",    0, 0, total_x,          total_y + dy,     w, h)
E.add_tile("nico7-10",    0, 0, total_x,          total_y + dy * 2, w, h)
E.add_tile("nico8-10",    0, 0, total_x,          total_y + dy * 3, w, h)

E.add_tile("nico5-10", 1600, 0, total_x + dx,     total_y,          w, h)
E.add_tile("nico6-10", 1600, 0, total_x + dx,     total_y + dy,     w, h)
E.add_tile("nico7-10", 1600, 0, total_x + dx,     total_y + dy * 2, w, h)
E.add_tile("nico8-10", 1600, 0, total_x + dx,     total_y + dy * 3, w, h)

E.add_tile("nico1-10",    0, 0, total_x + dx * 2, total_y,          w, h)
E.add_tile("nico2-10",    0, 0, total_x + dx * 2, total_y + dy,     w, h)
E.add_tile("nico3-10",    0, 0, total_x + dx * 2, total_y + dy * 2, w, h)
E.add_tile("nico4-10",    0, 0, total_x + dx * 2, total_y + dy * 3, w, h)

E.add_tile("nico1-10", 1600, 0, total_x + dx * 3, total_y,          w, h)
E.add_tile("nico2-10", 1600, 0, total_x + dx * 3, total_y + dy,     w, h)
E.add_tile("nico3-10", 1600, 0, total_x + dx * 3, total_y + dy * 2, w, h)
E.add_tile("nico4-10", 1600, 0, total_x + dx * 3, total_y + dy * 3, w, h)
