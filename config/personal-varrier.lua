
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

l = { 266.67083, -7.572, 0.0076, 0.0003, 0.8125 }

offset = { 0, (p[2][2] + p[1][2]) / 2, 2.0 + p[1][3] }

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_position(tile, p[1][1], p[1][2], p[1][3],
                    p[3][1] - p[1][1], p[3][2] - p[1][2], p[3][3] - p[1][3],
                    p[2][1] - p[1][1], p[2][2] - p[1][2], p[2][3] - p[1][3])
E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])

E.set_host_flags(host, E.host_flag_framed, false)

E.set_background(1, 0, 0, 0, 1, 0)

E.set_tile_view_offset(tile, 0, 4, 0)

-------------------------------------------------------------------------------

function varrier_dump()
    print("l = { "..l[1]..", "..l[2]..", "..l[3]..", "..l[4]..", "..l[5].." }")
end

function varrier_thick(d)
    l[3] = l[3] + d;
    E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])
    varrier_dump()
    return true
end

function varrier_shift(d)
    l[4] = l[4] + d;
    E.set_tile_line_screen(tile, l[1], l[2], l[3], l[4], l[5])
    varrier_dump()
    return true
end

------------------------------------------------------------------------------

function varrier_keyboard(k, s, camera)
    local dx =  2.50 / 12.0 * 0.5
    local dy = -1.23 / 12.0
    local dz =  2.00 / 12.0

    if s then
        if E.get_modifier(E.key_modifier_control) then

            if k == E.key_down  then return varrier_thick(-0.0001) end
            if k == E.key_up    then return varrier_thick( 0.0001) end
            if k == E.key_left  then return varrier_shift(-0.0001) end
            if k == E.key_right then return varrier_shift( 0.0001) end

            if k == E.key_tab then
                test = not test
                E.set_tile_flags(tile, E.tile_flag_test, test)
                return true
            end
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
    else
        if k == E.key_down  then return true end
        if k == E.key_up    then return true end
        if k == E.key_left  then return true end
        if k == E.key_right then return true end
    end
    return false
end

