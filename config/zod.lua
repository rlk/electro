
-------------------------------------------------------------------------------

local_X = 100
local_Y = 100
local_w = 200
local_h = 150

dx = local_w + 10  -- Tile width  including mullion
dy = local_h + 10  -- Tile height including mullion

total_w  =  dx * 2
total_h  =  dy * 2
total_x  = -total_w / 2
total_y  = -total_h / 2

E.add_tile("zod", local_X +  0, local_Y +  0,
                  total_x +  0, total_y +  0, local_w, local_h)
E.add_tile("zod", local_X + dx, local_Y +  0,
                  total_x + dx, total_y +  0, local_w, local_h)
E.add_tile("zod", local_X +  0, local_Y + dy,
                  total_x +  0, total_y + dy, local_w, local_h)
E.add_tile("zod", local_X + dx, local_Y + dy,
                  total_x + dx, total_y + dy, local_w, local_h)
