
W =  2.0000
H =  1.1250
D =  2.0000
O =  0.2083

w = 2560
h = 1600

p = {
    { -1.0521, 3.1354, -1.3614 },
    { -1.0521, 4.4531, -1.3614 },
    {  1.0521, 3.1354, -1.3614 },
}

-- l = { 266.670830, -7.572000, 0.016700, 0.000000, 0.81250 }

l = { 266.67083, -7.572, 0.0117, -0.0008, 0.8125 }

offset = { 0, (p[2][2] + p[1][2]) / 2, 2.0 + p[1][3] }

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h + 1200)
tile = E.add_tile(host,      0, 1200, w, h)

E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_position(tile, p[1][1], p[1][2], p[1][3],
                    p[3][1] - p[1][1], p[3][2]- p[1][2], p[3][3] - p[1][3],
                    p[2][1] - p[1][1], p[2][2]- p[1][2], p[2][3] - p[1][3])
E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])

--E.set_host_flags(host, E.host_flag_full, true)
E.set_host_flags(host, E.host_flag_framed, false)

E.set_background(1, 0, 0, 0, 1, 0)

-------------------------------------------------------------------------------

function varrier_dump()
    print("l = { "..l[1]..", "..l[2]..", "..l[3]..", "..l[4]..", "..l[5].." }")
end

function varrier_pitch(d)
    l[1] = l[1] + d;
    E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])
end

function varrier_angle(d)
    l[2] = l[2] + d;
    E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])
end

function varrier_thick(d)
    l[3] = l[3] + d;
    E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])
end

function varrier_shift(d)
    l[4] = l[4] + d;
    E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])
end

-------------------------------------------------------------------------------

varrier_func = varrier_shift
varrier_diff = 0.0001
varrier_test = false

function set_varrier_value(d)
    varrier_func(d * varrier_diff)
    varrier_dump()
    return true
end

function set_varrier_func(func, name)
    varrier_func = func
--  E.print_console("variable = "..name.."\n")
    return true
end

function set_varrier_diff(diff)
    varrier_diff = diff
--  E.print_console("value = "..diff.."\n")
    return true
end

function tog_varrier_test()
    varrier_test = not varrier_test

    E.set_tile_flags(tile, E.tile_flag_test, varrier_test)
end

------------------------------------------------------------------------------

function varrier_keyboard(k, s, camera)
    local dx =  2.50 / 12.0 * 0.5
    local dy = -1.23 / 12.0
    local dz =  2.00 / 12.0

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

            if k == E.key_tab   then return tog_varrier_test() end
        end

        if k == E.key_F5 then
            E.set_camera_stereo(camera, E.stereo_mode_none,
                                0, 0, 0, 0, 0, 0)
            return true
        end
        if k == E.key_F6 then
            E.set_camera_stereo(camera, E.stereo_mode_varrier_01,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == E.key_F11 then
            E.set_camera_stereo(camera, E.stereo_mode_varrier_11,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == E.key_F12 then
            E.set_camera_stereo(camera, E.stereo_mode_varrier_33,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
    end
    return false
end

