
w = 1600
h = 1200

x = { -2, -1, 0, 1 }
y = { -2, -1, 0, 1 }
z   = -2

R = { 0.92, 0, 0 }
U = { 0, 0.92, 0 }

E.add_host("server", 0, 0, 800, 600)
E.add_tile("server", 0, 0, 800, 600, { -2, -2, -2 }, { 4, 0, 0 }, { 0, 4, 0 })

E.add_host("nico1-10", 0, 0, w * 2, h)
E.add_tile("nico1-10", 0, 0, w, h, { x[3], y[4], z }, R, U)
E.add_tile("nico1-10", w, 0, w, h, { x[4], y[4], z }, R, U)

E.add_host("nico2-10", 0, 0, w * 2, h)
E.add_tile("nico2-10", 0, 0, w, h, { x[3], y[3], z }, R, U)
E.add_tile("nico2-10", w, 0, w, h, { x[4], y[3], z }, R, U)

E.add_host("nico3-10", 0, 0, w * 2, h)
E.add_tile("nico3-10", 0, 0, w, h, { x[3], y[2], z }, R, U)
E.add_tile("nico3-10", w, 0, w, h, { x[4], y[2], z }, R, U)

E.add_host("nico4-10", 0, 0, w * 2, h)
E.add_tile("nico4-10", 0, 0, w, h, { x[3], y[1], z }, R, U)
E.add_tile("nico4-10", w, 0, w, h, { x[4], y[1], z }, R, U)

E.add_host("nico5-10", 0, 0, w * 2, h)
E.add_tile("nico5-10", 0, 0, w, h, { x[1], y[4], z }, R, U)
E.add_tile("nico5-10", w, 0, w, h, { x[2], y[4], z }, R, U)

E.add_host("nico6-10", 0, 0, w * 2, h)
E.add_tile("nico6-10", 0, 0, w, h, { x[1], y[3], z }, R, U)
E.add_tile("nico6-10", w, 0, w, h, { x[2], y[3], z }, R, U)

E.add_host("nico7-10", 0, 0, w * 2, h)
E.add_tile("nico7-10", 0, 0, w, h, { x[1], y[2], z }, R, U)
E.add_tile("nico7-10", w, 0, w, h, { x[2], y[2], z }, R, U)

E.add_host("nico8-10", 0, 0, w * 2, h)
E.add_tile("nico8-10", 0, 0, w, h, { x[1], y[1], z }, R, U)
E.add_tile("nico8-10", w, 0, w, h, { x[2], y[1], z }, R, U)

-------------------------------------------------------------------------------
--[[
mullion_w = 128
mullion_h = 128

w  = 1600
h  = 1200
dx = w + mullion_w
dy = h + mullion_h

total_w  =  dx * 4
total_h  =  dy * 4
total_x  = -total_w / 2 + mullion_w / 2
total_y  = -total_h / 2 + mullion_h / 2

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
]]--