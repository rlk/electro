-- Varrier configuration is done in two parts: the display-specific config,
-- and the general Varrier config.  This is the general Varrier config.

-- This script expects Varrier line screen parameters to be defined by a table
-- of tables named "line_screen" where each table gives the parameters for
-- one tile in the order { pitch, angle, thickness, shift, duty cycle }.

-- This script also expects a de-facto Electro configuration where host IDs
-- are given by a table named "host" with index 0 being the server host and
-- 1 through n being the n client hosts.  Tile IDs should be given by a table
-- named "tile" with index 0 being the server tile and 1 through m being the m
-- displays.

varrier_tile = 0
varrier_qual = 0.5
varrier_test = false

-------------------------------------------------------------------------------

-- Parse and extract a screen from Tom Peterka's linescreen definition file.

function read_line_screen_file(file, screen, i)
    for line in io.lines(file) do
        local b, e, name, p, a, t, s, c =
            string.find(line, "(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)")

        if name == screen then
            line_screen[i][1] = p
            line_screen[i][2] = a
            line_screen[i][3] = t
            line_screen[i][4] = s
            line_screen[i][5] = c
        end
    end
end

-- Write Lua code giving the current line screen configuration.

function dump_line_screen()
    local i

    print("line_screen = {")

    for i = 0, table.getn(line_screen) do
        if tile[i] then
            print("    ["..i.."] = { "..line_screen[i][1]..", "
                                      ..line_screen[i][2]..", "
                                      ..line_screen[i][3]..", "
                                      ..line_screen[i][4]..", "
                                      ..line_screen[i][5].." },")
        end
    end

    print("}")
end

-------------------------------------------------------------------------------

-- Adjust a line screen value.

function set_line_screen(i)
    E.set_tile_line_screen(tile[i], line_screen[i][1],
                                    line_screen[i][2],
                                    line_screen[i][3],
                                    line_screen[i][4],
                                    line_screen[i][5])
end

function set_line_screen_thick(i, k)
    line_screen[i][3] = line_screen[i][3] + k
    set_line_screen(i)
end

function set_line_screen_shift(i, k)
    line_screen[i][4] = line_screen[i][4] + k
    set_line_screen(i)
end

-------------------------------------------------------------------------------

-- Adjust the optical thickness of one or all Varrier tiles.

function set_varrier_thick(d)
    local i

    if varrier_tile == 0 then
        for i = 0, table.getn(line_screen) do
            if tile[i] then
                set_line_screen_thick(i, d)
            end
        end
    else
        set_line_screen_thick(varrier_tile, d)
    end

    return true
end

-- Adjust the shift of one or all Varrier tiles.

function set_varrier_shift(d)
    local i

    if varrier_tile == 0 then
        for i = 0, table.getn(line_screen) do
            if tile[i] then
                set_line_screen_shift(i, d)
            end
        end
    else
        set_line_screen_shift(varrier_tile, d)
    end

    return true
end

-- Toggle the test state of one or all Varrier tiles.

function set_varrier_test(b)
    local i

    varrier_test = b

    if varrier_tile == 0 then
        for i = 0, table.getn(line_screen) do
            if tile[i] then
                E.set_tile_flags(tile[i], E.tile_flag_test, varrier_test)
            end
        end
    else
        E.set_tile_flags(tile[varrier_tile], E.tile_flag_test, varrier_test)
    end

    return true
end

-- Adjust the Varrier Combiner quality setting of all tiles.

function set_varrier_qual(d)
    local i

    varrier_qual = varrier_qual + d

    if varrier_qual < 0.0 then varrier_qual = 0.0 end
    if varrier_qual > 1.0 then varrier_qual = 1.0 end

    for i = 0, table.getn(line_screen) do
        if tile[i] then
            E.set_tile_quality(tile[i], varrier_qual, varrier_qual)
        end
    end

    return true
end

-- Cycle up or down through the set of Varrier tiles.  Index 0 denotes all.

function set_varrier_tile(d)
    local n = table.getn(line_screen)

    set_varrier_test(false)

    varrier_tile = varrier_tile + d

    if varrier_tile < 0 then varrier_tile = n end
    if varrier_tile > n then varrier_tile = 0 end

    set_varrier_test(true)

    return true
end

------------------------------------------------------------------------------

-- Process a Varrier keyboard event.

function varrier_keyboard(k, s, camera)
    local dx = 2.50 / 12.0 * 0.5
    local dy = 0
    local dz = 0

    if s then
        if E.get_modifier(E.key_modifier_control) then

            if k == E.key_insert   then return set_varrier_qual( 0.05) end
            if k == E.key_delete   then return set_varrier_qual(-0.05) end

            if k == E.key_pageup   then return set_varrier_tile( 1) end
            if k == E.key_pagedown then return set_varrier_tile(-1) end

            if k == E.key_down     then return set_varrier_thick(-0.0001) end
            if k == E.key_up       then return set_varrier_thick( 0.0001) end

            if k == E.key_left     then return set_varrier_shift(-0.00005) end
            if k == E.key_right    then return set_varrier_shift( 0.00005) end

            if k == E.key_tab then
                set_varrier_test(not varrier_test)

                if not varrier_test then
                    dump_line_screen()
                end

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
            if k == E.key_up       then return true end
            if k == E.key_down     then return true end
            if k == E.key_left     then return true end
            if k == E.key_right    then return true end
            if k == E.key_insert   then return true end
            if k == E.key_delete   then return true end
            if k == E.key_pageup   then return true end
            if k == E.key_pagedown then return true end
        end
    end
    return false
end

-- Initialize Varrier line screen and Combiner quality settings.

function varrier_init()
    local i

    for i = 0, table.getn(line_screen) do
        if tile[i] then
            set_line_screen(i)
        end
    end

    set_varrier_qual(0)
end
