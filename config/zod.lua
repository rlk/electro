
E.add_host("zod",   0,   0, 800, 300)
E.add_tile("zod",   0,   0, 400, 300, { -1, -0.375, -2 }, { 1, 0, 0 }, { 0, 0.75, 0 })
E.add_tile("zod", 400,   0, 400, 300, {  0, -0.375, -2 }, { 1, 0, 0 }, { 0, 0.75, 0 })

-------------------------------------------------------------------------------
--[[
mullion_w = 10
mullion_h = 10

local_X = 100
local_Y = 100
local_w = 200
local_h = 150

dx = local_w + mullion_w
dy = local_h + mullion_h

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

]]--