
-------------------------------------------------------------------------------

w  = 1600
h  = 1200
dx = w + 128  -- Tile width including mullion
dy = h + 128  -- Tile height including mullion

global_w  =  dx * 4
global_h  =  dy * 4
global_x  = -global_w / 2
global_y  = -global_h / 2

add_tile("nico5-10",    0, 0, global_x,          global_y,          w, h)
add_tile("nico6-10",    0, 0, global_x,          global_y + dy,     w, h)
add_tile("nico7-10",    0, 0, global_x,          global_y + dy * 2, w, h)
add_tile("nico8-10",    0, 0, global_x,          global_y + dy * 3, w, h)

add_tile("nico5-10", 1600, 0, global_x + dx,     global_y,          w, h)
add_tile("nico6-10", 1600, 0, global_x + dx,     global_y + dy,     w, h)
add_tile("nico7-10", 1600, 0, global_x + dx,     global_y + dy * 2, w, h)
add_tile("nico8-10", 1600, 0, global_x + dx,     global_y + dy * 3, w, h)

add_tile("nico1-10",    0, 0, global_x + dx * 2, global_y,          w, h)
add_tile("nico2-10",    0, 0, global_x + dx * 2, global_y + dy,     w, h)
add_tile("nico3-10",    0, 0, global_x + dx * 2, global_y + dy * 2, w, h)
add_tile("nico4-10",    0, 0, global_x + dx * 2, global_y + dy * 3, w, h)

add_tile("nico1-10", 1600, 0, global_x + dx * 3, global_y,          w, h)
add_tile("nico2-10", 1600, 0, global_x + dx * 3, global_y + dy,     w, h)
add_tile("nico3-10", 1600, 0, global_x + dx * 3, global_y + dy * 2, w, h)
add_tile("nico4-10", 1600, 0, global_x + dx * 3, global_y + dy * 3, w, h)
