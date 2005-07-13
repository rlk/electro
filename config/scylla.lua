host = { }
tile = { }

-- 19 hosts: 1 server, 18 clients.

host[0]  = E.add_host("scylla.evl.uic.edu",      0, 0, 1600, 1200)
host[1]  = E.add_host("scylla1-10.evl.uic.edu",  0, 0, 1600, 2400)
host[2]  = E.add_host("scylla2-10.evl.uic.edu",  0, 0, 1600, 2400)
host[3]  = E.add_host("scylla3-10.evl.uic.edu",  0, 0, 1600, 2400)
host[4]  = E.add_host("scylla4-10.evl.uic.edu",  0, 0, 1600, 2400)
host[5]  = E.add_host("scylla5-10.evl.uic.edu",  0, 0, 1600, 2400)
host[6]  = E.add_host("scylla6-10.evl.uic.edu",  0, 0, 1600, 2400)
host[7]  = E.add_host("scylla7-10.evl.uic.edu",  0, 0, 1600, 2400)
host[8]  = E.add_host("scylla8-10.evl.uic.edu",  0, 0, 1600, 2400)
host[9]  = E.add_host("scylla9-10.evl.uic.edu",  0, 0, 1600, 2400)
host[10] = E.add_host("scylla10-10.evl.uic.edu", 0, 0, 1600, 2400)
host[11] = E.add_host("scylla11-10.evl.uic.edu", 0, 0, 1600, 2400)
host[12] = E.add_host("scylla12-10.evl.uic.edu", 0, 0, 3200, 1200)
host[13] = E.add_host("scylla13-10.evl.uic.edu", 0, 0, 3200, 1200)
host[14] = E.add_host("scylla14-10.evl.uic.edu", 0, 0, 3200, 1200)
host[15] = E.add_host("scylla15-10.evl.uic.edu", 0, 0, 1600, 1200)
host[16] = E.add_host("scylla16-10.evl.uic.edu", 0, 0, 1600, 2400)
host[17] = E.add_host("scylla17-10.evl.uic.edu", 0, 0, 1600, 2400)
host[18] = E.add_host("scylla18-10.evl.uic.edu", 0, 0, 1600, 2400)

E.set_host_flag(host[0], E.host_flag_framed, false);

-- Tile host numbers.

num = {
   host[1],  host[2],  host[3],  host[4],  host[5],  host[6],  host[7],
   host[1],  host[2],  host[3],  host[4],  host[5],  host[6],  host[7],
   host[8],  host[9],  host[16], host[17], host[18], host[10], host[11],
   host[8],  host[9],  host[16], host[17], host[18], host[10], host[11],
   host[12], host[12], host[13], host[13], host[14], host[14], host[15],
}

-- Mapping to Todd's crazy tile numbering scheme.

map = {
     1,  3,  5,  7,  9, 11, 13,
     2,  4,  6,  8, 10, 12, 14,
    15, 17, 30, 32, 34, 19, 21,
    16, 18, 31, 33, 35, 20, 22,
    23, 24, 25, 26, 27, 28, 29
}

-- Tile sub-windows.

w = {
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0, 1200, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {   1600,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {   1600,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 },
   {   1600,    0, 1600, 1200 },
   {      0,    0, 1600, 1200 }
}

-- Tile viewports.

v = {
   {      0,    0, 1600, 1200 },
   {   1728,    0, 1600, 1200 },
   {   3456,    0, 1600, 1200 },
   {   5184,    0, 1600, 1200 },
   {   6912,    0, 1600, 1200 },
   {   8640,    0, 1600, 1200 },
   {  10368,    0, 1600, 1200 },
   {      0, 1328, 1600, 1200 },
   {   1728, 1328, 1600, 1200 },
   {   3456, 1328, 1600, 1200 },
   {   5184, 1328, 1600, 1200 },
   {   6912, 1328, 1600, 1200 },
   {   8640, 1328, 1600, 1200 },
   {  10368, 1328, 1600, 1200 },
   {      0, 2656, 1600, 1200 },
   {   1728, 2656, 1600, 1200 },
   {   3456, 2656, 1600, 1200 },
   {   5184, 2656, 1600, 1200 },
   {   6912, 2656, 1600, 1200 },
   {   8640, 2656, 1600, 1200 },
   {  10368, 2656, 1600, 1200 },
   {      0, 3984, 1600, 1200 },
   {   1728, 3984, 1600, 1200 },
   {   3456, 3984, 1600, 1200 },
   {   5184, 3984, 1600, 1200 },
   {   6912, 3984, 1600, 1200 },
   {   8640, 3984, 1600, 1200 },
   {  10368, 3984, 1600, 1200 },
   {      0, 5312, 1600, 1200 },
   {   1728, 5312, 1600, 1200 },
   {   3456, 5312, 1600, 1200 },
   {   5184, 5312, 1600, 1200 },
   {   6912, 5312, 1600, 1200 },
   {   8640, 5312, 1600, 1200 },
   {  10368, 5312, 1600, 1200 }
}

-- Tile screen locations.

p = {
   { -3.9891,  1.5361, -2.6218,  0.7527,  0.0, -1.1006,  0.0,  1.0,  0.0 },
   { -3.1792,  1.5386, -3.7931,  1.0776,  0.0, -0.7852,  0.0,  1.0,  0.0 },
   { -2.0212,  1.5401, -4.6122,  1.2680,  0.0, -0.4123,  0.0,  1.0,  0.0 },
   { -0.6638,  1.5438, -5.0334,  1.3334,  0.0,  0.0027,  0.0,  1.0,  0.0 },
   {  0.7556,  1.5611, -5.0259,  1.2530,  0.0,  0.4557,  0.0,  1.0,  0.0 },
   {  2.0884,  1.5654, -4.5348,  1.0352,  0.0,  0.8404,  0.0,  1.0,  0.0 },
   {  3.1862,  1.5621, -3.6356,  0.7185,  0.0,  1.1231,  0.0,  1.0,  0.0 },
   { -3.9842,  2.6294, -2.6195,  0.7467,  0.0, -1.1046,  0.0,  1.0,  0.0 },
   { -3.1783,  2.6302, -3.7900,  1.0761,  0.0, -0.7873,  0.0,  1.0,  0.0 },
   { -2.0205,  2.6312, -4.6105,  1.2671,  0.0, -0.4151,  0.0,  1.0,  0.0 },
   { -0.6620,  2.6390, -5.0315,  1.3333,  0.0,  0.0050,  0.0,  1.0,  0.0 },
   {  0.7567,  2.6536, -5.0211,  1.2498,  0.0,  0.4645,  0.0,  1.0,  0.0 },
   {  2.0893,  2.6558, -4.5266,  1.0388,  0.0,  0.8358,  0.0,  1.0,  0.0 },
   {  3.1893,  2.6541, -3.6264,  0.7206,  0.0,  1.1218,  0.0,  1.0,  0.0 },
   { -3.9886,  3.7210, -2.6214,  0.7518,  0.0, -1.1012,  0.0,  1.0,  0.0 },
   { -3.1813,  3.7208, -3.7934,  1.0798,  0.0, -0.7821,  0.0,  1.0,  0.0 },
   { -2.0230,  3.7226, -4.6139,  1.2709,  0.0, -0.4032,  0.0,  1.0,  0.0 },
   { -0.6619,  3.7318, -5.0181,  1.3333,  0.0,  0.0000,  0.0,  1.0,  0.0 },--
   {  0.7581,  3.7457, -5.0147,  1.2520,  0.0,  0.4586,  0.0,  1.0,  0.0 },
   {  2.0872,  3.7471, -4.5151,  1.0366,  0.0,  0.8385,  0.0,  1.0,  0.0 },
   {  3.1886,  3.7448, -3.6154,  0.7202,  0.0,  1.1220,  0.0,  1.0,  0.0 },
   { -3.9918,  4.8125, -2.6237,  0.7455,  0.0, -1.1054,  0.0,  1.0,  0.0 },
   { -3.1849,  4.8130, -3.8003,  1.0812,  0.0, -0.7803,  0.0,  1.0,  0.0 },
   { -2.0225,  4.8170, -4.6155,  1.2725,  0.0, -0.3979,  0.0,  1.0,  0.0 },
   { -0.6643,  4.8243, -5.0305,  1.3333,  0.0,  0.0091,  0.0,  1.0,  0.0 },
   {  0.7588,  4.8375, -5.0089,  1.2505,  0.0,  0.4627,  0.0,  1.0,  0.0 },
   {  2.0885,  4.8391, -4.5062,  1.0383,  0.0,  0.8365,  0.0,  1.0,  0.0 },
   {  3.1906,  4.8353, -3.6077,  0.7135,  0.0,  1.1264,  0.0,  1.0,  0.0 },
   { -3.9993,  5.9056, -2.6255,  0.7564,  0.0, -1.0980,  0.0,  1.0,  0.0 },
   { -3.1853,  5.9030, -3.7941,  1.0835,  0.0, -0.7770,  0.0,  1.0,  0.0 },
   { -2.0194,  5.9113, -4.6041,  1.2710,  0.0, -0.4028,  0.0,  1.0,  0.0 },
   { -0.6648,  5.9167, -5.0286,  1.3333,  0.0,  0.0153,  0.0,  1.0,  0.0 },
   {  0.7598,  5.9308, -5.0105,  1.2486,  0.0,  0.4678,  0.0,  1.0,  0.0 },
   {  2.0904,  5.9317, -4.5006,  1.0336,  0.0,  0.8423,  0.0,  1.0,  0.0 },
   {  3.1963,  5.9276, -3.6040,  0.7051,  0.0,  1.1317,  0.0,  1.0,  0.0 }
}

-- Varrier line screen definitions.

pitch = 271.945865

l = {
    { pitch, -7.860000, 0.034800,  0.0066, 0.777777 }, -- 1
    { pitch, -7.800000, 0.035800, -0.0014, 0.777777 }, -- 2
    { pitch, -7.830000, 0.034900,  0.0050, 0.777777 }, -- 3
    { pitch, -7.770000, 0.034600,  0.0027, 0.777777 }, -- 4
    { pitch, -7.730000, 0.035300,  0.0006, 0.777777 }, -- 5
    { pitch, -7.830000, 0.035400, -0.0003, 0.777777 }, -- 6
    { pitch, -7.690000, 0.036200,  0.0018, 0.777777 }, -- 7
    { pitch, -7.770000, 0.035700,  0.0027, 0.777777 }, -- 8
    { pitch, -7.750000, 0.035000,  0.0003, 0.777777 }, -- 9
    { pitch, -7.741000, 0.035000,  0.0000, 0.777777 }, -- 10
    { pitch, -7.770000, 0.035200,  0.0011, 0.777777 }, -- 11
    { pitch, -7.850000, 0.035000,  0.0021, 0.777777 }, -- 12
    { pitch, -7.800000, 0.036000,  0.0020, 0.777777 }, -- 13
    { pitch, -7.770000, 0.036000,  0.0011, 0.777777 }, -- 14
    { pitch, -7.830000, 0.036700,  0.0021, 0.777777 }, -- 15 !
    { pitch, -7.790000, 0.026800,  0.0000, 0.777777 }, -- 16
    { pitch, -7.830000, 0.036400,  0.0041, 0.777777 }, -- 17
    { pitch, -7.750000, 0.035700,  0.0038, 0.777777 }, -- 18
    { pitch, -7.845000, 0.038620,  0.0031, 0.777777 }, -- 19 !
    { pitch, -7.782000, 0.037200,  0.0043, 0.777777 }, -- 20
    { pitch, -7.890000, 0.036700,  0.0034, 0.777777 }, -- 21
    { pitch, -7.880000, 0.037000,  0.0037, 0.777777 }, -- 22
    { pitch, -7.800000, 0.036500,  0.0000, 0.777777 }, -- 23
    { pitch, -7.830000, 0.035700,  0.0029, 0.777777 }, -- 24
    { pitch, -7.801000, 0.036400, -0.0009, 0.777777 }, -- 25
    { pitch, -7.820000, 0.036200,  0.0034, 0.777777 }, -- 26
    { pitch, -7.770000, 0.036000,  0.0024, 0.777777 }, -- 27
    { pitch, -7.830000, 0.036000,  0.0040, 0.777777 }, -- 28
    { pitch, -7.880000, 0.037000,  0.0025, 0.777777 }, -- 29
    { pitch, -7.780000, 0.036000, -0.0017, 0.777777 }, -- 30
    { pitch, -7.730000, 0.034800,  0.0011, 0.777777 }, -- 31
    { pitch, -7.760000, 0.037000, -0.0020, 0.777777 }, -- 32 !
    { pitch, -7.740000, 0.035800,  0.0038, 0.777777 }, -- 33
    { pitch, -7.890000, 0.036000,  0.0031, 0.777777 }, -- 34
    { pitch, -7.810000, 0.036400,  0.0012, 0.777777 }  -- 35
--    pitch     angle   thickness   shift    cycle
}

-- Mirror the center tile on the server.

num[0] = 0
map[0] = 0

l[0] = { 271.945865, -7.750000, 0.035700,  0.0038, 0.777777 }
w[0] = w[18]
v[0] = v[18]
p[0] = p[18]

for i = 0, 35 do
--    l[i][1] = 1
--    l[i][2] = 0
--    l[i][3] = 0
--    l[i][4] = 0
end

-------------------------------------------------------------------------------

-- Configure all hosts.

for i = 0, 35 do
    local k = map[i]

    tile[i] = E.add_tile(num[i], w[i][1], w[i][2], w[i][3], w[i][4])

    E.set_tile_viewport(tile[i], v[i][1], v[i][2], v[i][3], v[i][4])
    E.set_tile_position(tile[i], p[i][1], p[i][2], p[i][3], p[i][4],
                        p[i][5], p[i][6], p[i][7], p[i][8], p[i][9])

    E.set_tile_line_screen(tile[i], l[k][1], l[k][2],
                           l[k][3], l[k][4], l[k][5])
end

-------------------------------------------------------------------------------

function varrier_dump()
    for i = 1, 35 do
        local k = map[i]
        print("{ "..l[k][1]..", "..l[k][2]..", "..l[k][3]..
               ","..l[k][4]..", "..l[k][5].." }, -- "..k)
    end
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
        E.set_tile_flag(tile[varrier_tile], E.tile_flag_test, varrier_test)
    else
        for i = 0, 35 do
            E.set_tile_flag(tile[i], E.tile_flag_test, varrier_test)
        end
    end
end

-------------------------------------------------------------------------------

function chr(s)
    return string.byte(s)
end

function varrier_keyboard(k, s, camera)
    local dx =  2.50 / 12.0 * 0.5
    local dy = -1.23 / 12.0
    local dz =  2.00 / 12.0

    if s then
        if E.get_modifier(E.key_modifier_control) then
            if k == chr("1") then return set_varrier_diff(10.0) end
            if k == chr("2") then return set_varrier_diff(1.0) end
            if k == chr("3") then return set_varrier_diff(0.1) end
            if k == chr("4") then return set_varrier_diff(0.01) end
            if k == chr("5") then return set_varrier_diff(0.001) end
            if k == chr("6") then return set_varrier_diff(0.0001) end
            if k == chr("7") then return set_varrier_diff(0.00001) end
            if k == chr("8") then return set_varrier_diff(0.000001) end

            if k == chr("s") then
                return set_varrier_func(varrier_shift, "shift")
            end
            if k == chr("t") then
                return set_varrier_func(varrier_thick, "thick")
            end
            if k == chr("p") then
                return set_varrier_func(varrier_pitch, "pitch")
            end
            if k == chr("a") then
                return set_varrier_func(varrier_angle, "angle")
            end
            if k == chr("c") then
                return set_varrier_func(varrier_cycle, "cycle")
            end

            if k == 274 then return set_varrier_value(-1) end
            if k == 273 then return set_varrier_value( 1) end

            if k == 276 then return set_varrier_tile(-1) end
            if k == 275 then return set_varrier_tile( 1) end

            if k == chr("\t") then return tog_varrier_test() end
        end

        if k == 286 then -- F5
            E.set_entity_frag_prog(scene, nil)
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_none,
                                0, 0, 0, 0, 0, 0)
            return true
        end
        if k == 287 then -- F6
            E.set_entity_frag_prog(scene, nil)
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_varrier_01,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == 288 then -- F7
            E.set_entity_frag_prog(scene, "../varrier-01-frag.fp")
            E.set_entity_vert_prog(scene, "../varrier-01-vert.vp")
            E.set_camera_stereo(camera, E.stereo_mode_varrier_01,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == 289 then -- F8
            E.set_entity_frag_prog(scene, "../varrier-01-fntx.fp")
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_varrier_01,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == 290 then -- F9
            E.set_entity_frag_prog(scene, "../varrier-01-both.fp")
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_varrier_01,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == 291 then -- F10
            E.set_entity_frag_prog(scene, "../varrier-01-bntx.fp")
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_varrier_01,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == 292 then -- F11
            E.set_entity_frag_prog(scene, nil)
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_varrier_11,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
        if k == 293 then -- F12
            E.set_entity_frag_prog(scene, nil)
            E.set_entity_vert_prog(scene, nil)
            E.set_camera_stereo(camera, E.stereo_mode_varrier_33,
                                -dx, dy, dz, dx, dy, dz)
            return true
        end
    end
    return false
end

function varrier_help()
    E.print_console(" F5: Select mono-scopic rendering\n")
    E.print_console(" F6: Select Varrier 0-1 fixed\n")
    E.print_console(" F7: Select Varrier 0-1 frag/vert\n")
    E.print_console(" F8: Select Varrier 0-1 fntx\n")
    E.print_console(" F9: Select Varrier 0-1 both\n")
    E.print_console("F10: Select Varrier 0-1 bntx\n")
    E.print_console("F11: Select Varrier 1-1\n")
    E.print_console("F12: Select Varrier 3-3\n")
end
