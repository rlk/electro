
-------------------------------------------------------------------------------

w  = 200
h  = 150
dx = w + 10  -- Tile width including mullion
dy = h + 10  -- Tile height including mullion

global_w  =  dx * 2
global_h  =  dy
global_x  = -global_w / 2
global_y  = -global_h / 2

local_x = 100
local_y = 100

E.add_tile("zod", local_x +  0, local_y, global_x,      global_y, w, h)
E.add_tile("zod", local_x + dx, local_y, global_x + dx, global_y, w, h)
