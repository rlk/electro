
local w = 400
local h = 300
local k = 0.950
local a = 0.750
local pR = { 1 * k, 0, 0 }
local pU = { 0, a * k, 0 }

E.add_host("zod", 0, 0, w * 2, h * 2)

E.add_tile("zod", 0, 0, w * k, h * k,
                  0, 0, w * k, h * k, { -1.0,  -a, -1.0 }, pR, pU)
E.add_tile("zod", w, 0, w * k, h * k,
                  w, 0, w * k, h * k, {  0.0,  -a, -1.0 }, pR, pU)
E.add_tile("zod", 0, h, w * k, h * k,
                  0, h, w * k, h * k, { -1.0, 0.0, -1.0 }, pR, pU)
E.add_tile("zod", w, h, w * k, h * k,
                  w, h, w * k, h * k, {  0.0, 0.0, -1.0 }, pR, pU)
