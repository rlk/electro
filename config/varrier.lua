
linescreen = {
    p = 270.807188,
    a =  -7.654000,
    t =   0.015100,
    s =   0.002290,
    c =   0.777777
}

h_quality = 1.0
v_quality = 1.0

-------------------------------------------------------------------------------

function read_linescreen(file, screen)
    for line in io.lines(file) do
        local b, e, name, p, a, t, s, c =
            string.find(line, "(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)")

        if name == screen then
            linescreen.p = p
            linescreen.a = a
            linescreen.t = t
            linescreen.s = s
            linescreen.c = c
        end
    end
end

-------------------------------------------------------------------------------

function varrier_thick(d)
    linescreen.t = linescreen.t + d;
    E.set_tile_line_screen(tile, linescreen.p, linescreen.a,
                                 linescreen.t, linescreen.s, linescreen.c)
    return true
end

function varrier_shift(d)
    linescreen.s = linescreen.s + d;
    E.set_tile_line_screen(tile, linescreen.p, linescreen.a,
                                 linescreen.t, linescreen.s, linescreen.c)
    return true
end

function varrier_h_quality(d)
    if 0.0 <= h_quality + d and h_quality + d <= 1.0 then
        h_quality = h_quality + d
        E.set_tile_quality(tile, h_quality, v_quality)
        print("quality", h_quality, v_quality)
    end
    return true
end

function varrier_v_quality(d)
    if 0.0 <= v_quality + d and v_quality + d <= 1.0 then
        v_quality = v_quality + d
        E.set_tile_quality(tile, h_quality, v_quality)
        print("quality", h_quality, v_quality)
    end
    return true
end

------------------------------------------------------------------------------

function varrier_keyboard(k, s, camera)
    local dx = 2.50 / 12.0 * 0.5
    local dy = 0
    local dz = 0

    if s then
        if E.get_modifier(E.key_modifier_control) then

            if k == E.key_down   then return varrier_thick(-0.0001) end
            if k == E.key_up     then return varrier_thick( 0.0001) end
            if k == E.key_left   then return varrier_shift(-0.00005) end
            if k == E.key_right  then return varrier_shift( 0.00005) end

            if k == E.key_insert then return varrier_h_quality( 0.05) end
            if k == E.key_delete then return varrier_h_quality(-0.05) end
            if k == E.key_home   then return varrier_v_quality( 0.05) end
            if k == E.key_end    then return varrier_v_quality(-0.05) end

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
        if k == E.key_F9 then
            E.set_camera_stereo(camera, E.stereo_mode_varrier_00,
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

