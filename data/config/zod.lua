
-------------------------------------------------------------------------------

w  = 200
h  = 150
dx = w + 10  -- Tile width including mullion
dy = h + 10  -- Tile height including mullion

global_w  =  dx * 4
global_h  =  dy * 4
global_x  = -global_w / 2
global_y  = -global_h / 2

local_x = 100
local_y = 100


E.add_tile("zod", local_x +    0, local_y +    0, global_x,        global_y,        w, h)
E.add_tile("zod", local_x +    0, local_y +   dy, global_x,        global_y +   dy, w, h)
E.add_tile("zod", local_x +    0, local_y + 2*dy, global_x,        global_y + 2*dy, w, h)
E.add_tile("zod", local_x +    0, local_y + 3*dy, global_x,        global_y + 3*dy, w, h)

E.add_tile("zod", local_x +   dx, local_y +    0, global_x +   dx, global_y,        w, h)
E.add_tile("zod", local_x +   dx, local_y +   dy, global_x +   dx, global_y +   dy, w, h)
E.add_tile("zod", local_x +   dx, local_y + 2*dy, global_x +   dx, global_y + 2*dy, w, h)
E.add_tile("zod", local_x +   dx, local_y + 3*dy, global_x +   dx, global_y + 3*dy, w, h)

E.add_tile("zod", local_x + 2*dx, local_y +    0, global_x + 2*dx, global_y,        w, h)
E.add_tile("zod", local_x + 2*dx, local_y +   dy, global_x + 2*dx, global_y +   dy, w, h)
E.add_tile("zod", local_x + 2*dx, local_y + 2*dy, global_x + 2*dx, global_y + 2*dy, w, h)
E.add_tile("zod", local_x + 2*dx, local_y + 3*dy, global_x + 2*dx, global_y + 3*dy, w, h)

E.add_tile("zod", local_x + 3*dx, local_y +    0, global_x + 3*dx, global_y,        w, h)
E.add_tile("zod", local_x + 3*dx, local_y +   dy, global_x + 3*dx, global_y +   dy, w, h)
E.add_tile("zod", local_x + 3*dx, local_y + 2*dy, global_x + 3*dx, global_y + 2*dy, w, h)
E.add_tile("zod", local_x + 3*dx, local_y + 3*dy, global_x + 3*dx, global_y + 3*dy, w, h)

