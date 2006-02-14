host = { }
tile = { }

-- 4 hosts: 1 server, 3 clients.

host[0]  = E.add_host("default",       0, 0, 1280, 1024)
host[1]  = E.add_host("tvarrier1-10",  0, 0, 3200, 1200)
host[2]  = E.add_host("tvarrier2-10",  0, 0, 3200, 1200)
host[3]  = E.add_host("tvarrier3-10",  0, 0, 3200, 1200)

E.set_host_flags(host[0], E.host_flag_full,   true);
E.set_host_flags(host[0], E.host_flag_framed, false);

-- Tile host numbers.

num = {
   host[1],
   host[1],
   host[2],
   host[2],
   host[3],
   host[3]
}

num[0] = host[0];

-- Tile sub-windows.

w = {
   { 1600,    0, 1600, 1200 },
   {    0,    0, 1600, 1200 },
   { 1600,    0, 1600, 1200 },
   {    0,    0, 1600, 1200 },
   { 1600,    0, 1600, 1200 },
   {    0,    0, 1600, 1200 }
}

-- Tile viewports.

v = {
   {      0,    0, 1600, 1200 },
   {      0, 1328, 1600, 1200 },
   {   1728,    0, 1600, 1200 },
   {   1728, 1328, 1600, 1200 },
   {   3456,    0, 1600, 1200 },
   {   3456, 1328, 1600, 1200 },
}

-- Tile screen locations.

p = {
    {{ -2.0932, 4.9889, -5.0124 },
     { -2.0932, 5.9889, -5.0124 },
     { -0.7632, 4.9889, -5.0215 }}, -- upper left
    {{ -2.0965, 3.8930, -5.0178 },
     { -2.0965, 4.8930, -5.0178 },
     { -0.7665, 3.8930, -5.0362 }}, -- lower left
    {{ -0.6670, 4.9878, -5.0292 },
     { -0.6670, 5.9878, -5.0292 },
     {  0.6630, 4.9878, -5.0270 }}, -- upper center
    {{ -0.6696, 3.8936, -5.0314 },
     { -0.6696, 4.8936, -5.0314 },
     {  0.6604, 3.8936, -5.0237 }}, -- lower center
    {{  0.7599, 4.9937, -5.0320 },
     {  0.7599, 5.9937, -5.0320 },
     {  2.0899, 4.9937, -5.0229 }}, -- upper right
    {{  0.7562, 3.9000, -5.0195 },
     {  0.7562, 4.9000, -5.0195 },
     {  2.0862, 3.9000, -5.0280 }}  -- lower right
}

-- Varrier line screen definitions.

l = {
--    pitch        angle     thickness  shift     cycle
    { 268.426710, -7.740000, 0.035000,  0.000000, 0.750000 },
    { 268.426710, -7.800000, 0.035000,  0.000500, 0.750000 },
    { 268.426710, -7.800000, 0.035000, -0.000800, 0.750000 },
    { 268.426710, -7.830000, 0.035000, -0.001600, 0.750000 },
    { 268.426710, -7.810000, 0.034400,  0.000600, 0.750000 },
    { 268.426710, -7.820000, 0.034000, -0.001900, 0.750000 }
}

-- Mirror a tile on the server.

mirror = 2;

l[0] = l[mirror];
w[0] = w[mirror];
v[0] = v[mirror];
p[0] = p[mirror];

-------------------------------------------------------------------------------

-- Configure all tiles.

for i = 0, 6 do

    tile[i] = E.add_tile(num[i], w[i][1], w[i][2], w[i][3], w[i][4])
    E.set_tile_viewport(tile[i], v[i][1], v[i][2], v[i][3], v[i][4])

    E.set_tile_position(tile[i],
                        p[i][1][1],
                        p[i][1][2],
                        p[i][1][3],
                        p[i][2][1] - p[i][1][1],
                        p[i][2][2] - p[i][1][2],
                        p[i][2][3] - p[i][1][3],
                        p[i][3][1] - p[i][1][1],
                        p[i][3][2] - p[i][1][2],
                        p[i][3][3] - p[i][1][3]);

    E.set_tile_line_screen(tile[i], l[i][1], l[i][2],
                           l[i][3], l[i][4], l[i][5])
end

-------------------------------------------------------------------------------

function varrier_dump()
    print("l = {")
    for i = 1, 35 do
        print("    { "..l[i][1]..", "..l[i][2]..", "..l[i][3]..
                  ", "..l[i][4]..", "..l[i][5].." }, -- "..i)
    end
    print("}")
end

function varrier_pitch(d, i)
    local k = map[i]
    l[k][1] = l[k][1] + d;
    E.set_tile_line_screen(tile[i], l[k][1], l[k][2],
                           l[k][3], l[k][4], l[k][5])
end

function varrier_angle(d, i)
    local k = map[i]
    l[k][2] = l[k][2] + d;
    E.set_tile_line_screen(tile[i], l[k][1], l[k][2],
                           l[k][3], l[k][4], l[k][5])
end

function varrier_thick(d, i)
    local k = map[i]
    l[k][3] = l[k][3] + d;
    E.set_tile_line_screen(tile[i], l[k][1], l[k][2],
                           l[k][3], l[k][4], l[k][5])
end

function varrier_shift(d, i)
    local k = map[i]
    l[k][4] = l[k][4] + d;
    E.set_tile_line_screen(tile[i], l[k][1], l[k][2],
                           l[k][3], l[k][4], l[k][5])
end

-------------------------------------------------------------------------------

varrier_func = varrier_shift
varrier_diff = 0.0001
varrier_tile = -1
varrier_test = false

function set_varrier_value(d)
    if varrier_tile > 0 then
        varrier_func(d * varrier_diff, varrier_tile)
    else
        for i = 0, 35 do
            varrier_func(d * varrier_diff, i)
        end
    end

    varrier_dump()
    return true
end

function set_varrier_func(func, name)
    varrier_func = func
    E.print_console("variable = "..name.."\n")
    return true
end

function set_varrier_diff(diff)
    varrier_diff = diff
    E.print_console("value = "..diff.."\n")
    return true
end

function set_varrier_tile(d)
    varrier_tile = varrier_tile + d

    if varrier_tile <  0 then varrier_tile = 35 end
    if varrier_tile > 35 then varrier_tile =  0 end

    if varrier_tile == 0 then
        E.print_console("tile = ALL\n")
    else
        E.print_console("tile = "..varrier_tile.."\n")
    end

    return true
end

function tog_varrier_test()
    varrier_test = not varrier_test

    if varrier_tile > 0 then
        E.set_tile_flags(tile[varrier_tile], E.tile_flag_test, varrier_test)
    else
        for i = 0, 35 do
            E.set_tile_flags(tile[i], E.tile_flag_test, varrier_test)
        end
    end
end

-------------------------------------------------------------------------------

function varrier_keyboard(k, s, camera)
    local dx =  2.50 / 12.0 * 0.5
    local dy = -1.23 / 12.0
    local dz =  2.00 / 12.0

-- HACK
    E.set_camera_offset(camera, 0.0, 5.5, 0.0);

    if s then
        if E.get_modifier(E.key_modifier_control) then
            if k == E.key_1 then return set_varrier_diff(10.0) end
            if k == E.key_2 then return set_varrier_diff(1.0) end
            if k == E.key_3 then return set_varrier_diff(0.1) end
            if k == E.key_4 then return set_varrier_diff(0.01) end
            if k == E.key_5 then return set_varrier_diff(0.001) end
            if k == E.key_6 then return set_varrier_diff(0.0001) end
            if k == E.key_7 then return set_varrier_diff(0.00001) end
            if k == E.key_8 then return set_varrier_diff(0.000001) end

            if k == E.key_s then
                return set_varrier_func(varrier_shift, "shift")
            end
            if k == E.key_t then
                return set_varrier_func(varrier_thick, "thick")
            end
            if k == E.key_p then
                return set_varrier_func(varrier_pitch, "pitch")
            end
            if k == E.key_a then
                return set_varrier_func(varrier_angle, "angle")
            end
            if k == E.key_c then
                return set_varrier_func(varrier_cycle, "cycle")
            end

            if k == E.key_down  then return set_varrier_value(-1) end
            if k == E.key_up    then return set_varrier_value( 1) end

            if k == E.key_left  then return set_varrier_tile(-1) end
            if k == E.key_right then return set_varrier_tile( 1) end

            if k == E.key_tab   then return tog_varrier_test() end
        end

        if k == E.key_F5 then
            E.set_camera_stereo(camera, E.stereo_mode_none,
                                0, 0, 0, 0, 0, 0)
            return true
        end
        if k == E.key_F6 then
            E.set_camera_stereo(camera, E.stereo_mode_red_blue,
                                -dx, dy, dz, dx, dy, dz)
            return true
	end
        if k == E.key_F7 then
            E.set_camera_stereo(camera, E.stereo_mode_varrier_11,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == E.key_F8 then
            E.set_camera_stereo(camera, E.stereo_mode_varrier_33,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
    end
    return false
end

