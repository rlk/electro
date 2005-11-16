linescreen_file = "/DEMO/varrier/varrier_configs/linescreen_setup.txt"
read_linescreen(linescreen_file, "screen1")

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

offset = { 0, (p[2][2] + p[1][2]) / 2, 2.0 + p[1][3] }

------------------------------------------------------------------------------

host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_position(tile, p[1][1], p[1][2], p[1][3],
                    p[3][1] - p[1][1], p[3][2] - p[1][2], p[3][3] - p[1][3],
                    p[2][1] - p[1][1], p[2][2] - p[1][2], p[2][3] - p[1][3])
E.set_tile_line_screen(tile, linescreen.p, linescreen.a,
                             linescreen.t, linescreen.s, linescreen.c)

E.set_host_flags(host, E.host_flag_framed, false)

-- Uncomment this line when head tracking is disabled.
--E.set_tile_view_offset(tile, 0, 4, 0)
--E.set_tile_view_offset(tile, 0, 0, -1)

------------------------------------------------------------------------------
--[[
host = E.add_host("default", 0, 0, w, h)
tile = E.add_tile(host,      0, 0, w, h)

E.set_tile_viewport(tile, 0, 0, w, h)
E.set_tile_position(tile, p[1][1], p[1][2], p[1][3],
                    p[3][1] - p[1][1], p[3][2] - p[1][2], p[3][3] - p[1][3],
                    p[2][1] - p[1][1], p[2][2] - p[1][2], p[2][3] - p[1][3])
E.set_tile_line_screen(tile, linescreen[1], linescreen[2],
                             linescreen[3], linescreen[4], linescreen[5])

E.set_host_flags(host, E.host_flag_framed, false)

--E.set_background(1, 0, 0, 0, 1, 0)

-- Uncomment this line when head tracking is disabled.
--E.set_tile_view_offset(tile, 0, 4, 0)
]]--
-------------------------------------------------------------------------------
--[[
function varrier_store()
    local fd = io.open(linescreen_file, "w")
    fd:write(string.format("linescreen = { %f, %f, %f, %f, %f }\n",
                           linescreen[1], linescreen[2],
                           linescreen[3], linescreen[4], linescreen[5]))
    fd:close()
end

function varrier_thick(d)
    linescreen[3] = linescreen[3] + d;
    E.set_tile_line_screen(tile, linescreen[1], linescreen[2],
                                 linescreen[3], linescreen[4], linescreen[5])
    varrier_store()
    return true
end

function varrier_shift(d)
    linescreen[4] = linescreen[4] + d;
    E.set_tile_line_screen(tile, linescreen[1], linescreen[2],
                                 linescreen[3], linescreen[4], linescreen[5])
    varrier_store()
    return true
end
]]--
------------------------------------------------------------------------------
--[[
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
        if E.get_modifier(E.key_modifier_control) then
            if k == E.key_down  then return true end
            if k == E.key_up    then return true end
            if k == E.key_left  then return true end
            if k == E.key_right then return true end
        end
    end
    return false
end
]]--
