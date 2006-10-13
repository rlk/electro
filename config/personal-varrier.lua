eye_x =  2.50 / 12.0 * 0.5
eye_y =  0.00
eye_z =  0.00

host = { }
tile = { }

------------------------------------------------------------------------------

w = 2560
h = 1600

p = {
    { -1.0521, 3.1354, -1.3614 },
    { -1.0521, 4.4531, -1.3614 },
    {  1.0521, 3.1354, -1.3614 },
}

line_screen = {
    [0] = { 266.67083, -7.572, 0.016, -0.00013, 0.8125 },
}

------------------------------------------------------------------------------

host[0] = E.add_host("default", w, 0, w, h)
tile[0] = E.add_tile(host[0],   0, 0, w, h)

E.set_tile_viewport(tile[0], 0, 0, w, h)
E.set_tile_position(tile[0],
                    p[1][1], p[1][2], p[1][3],
                    p[3][1] - p[1][1], p[3][2] - p[1][2], p[3][3] - p[1][3],
                    p[2][1] - p[1][1], p[2][2] - p[1][2], p[2][3] - p[1][3])
varrier_init()

E.set_host_flags(host[0], E.host_flag_framed, false)

-- This transform is necessary when using net-direct face tracking.
-- It should be commented out when trackd is in use.
--[[
E.set_tracker_transform(0,  1.000,  0.000,  0.000, 0.000,
                            0.000,  0.979,  0.199, 0.000,
                            0.000, -0.199,  0.979, 0.000,
                           -0.458,  5.000, -1.600, 1.000)
]]--
