eye_x =  2.50 / 12.0 * 0.5
eye_y = -2.00 / 12.0
eye_z =  1.00 / 12.0

host = { }
tile = { }

-- 19 hosts: 1 server, 18 clients.

host[0]  = E.add_host("default",     0, 0, 800,  600)
host[1]  = E.add_host("scylla1-10",  0, 0, 1600, 2400)
host[2]  = E.add_host("scylla2-10",  0, 0, 1600, 2400)
host[3]  = E.add_host("scylla3-10",  0, 0, 1600, 2400)
host[4]  = E.add_host("scylla4-10",  0, 0, 1600, 2400)
host[5]  = E.add_host("scylla5-10",  0, 0, 1600, 2400)
host[6]  = E.add_host("scylla6-10",  0, 0, 1600, 2400)
host[7]  = E.add_host("scylla7-10",  0, 0, 1600, 2400)
host[8]  = E.add_host("scylla8-10",  0, 0, 1600, 2400)
host[9]  = E.add_host("scylla9-10",  0, 0, 1600, 2400)
host[10] = E.add_host("scylla10-10", 0, 0, 1600, 2400)
host[11] = E.add_host("scylla11-10", 0, 0, 1600, 2400)
host[12] = E.add_host("scylla12-10", 0, 0, 3200, 1200)
host[13] = E.add_host("scylla13-10", 0, 0, 3200, 1200)
host[14] = E.add_host("scylla14-10", 0, 0, 3200, 1200)
host[15] = E.add_host("scylla15-10", 0, 0, 1600, 1200)
host[16] = E.add_host("scylla16-10", 0, 0, 1600, 2400)
host[17] = E.add_host("scylla17-10", 0, 0, 1600, 2400)
host[18] = E.add_host("scylla18-10", 0, 0, 1600, 2400)

E.set_host_flags(host[0], E.host_flag_framed, true);

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

line_screen = {
    [0] = { 271.945865, -7.76, 0.035, 0.0044, 0.777777 },
    [1] = { 271.945865, -7.86, 0.0328, 0.0096, 0.777777 },
    [2] = { 271.945865, -7.83, 0.0339, 0.0076, 0.777777 },
    [3] = { 271.945865, -7.73, 0.0343, 0.0071, 0.777777 },
    [4] = { 271.945865, -7.69, 0.0352, 0.0078, 0.777777 },
    [5] = { 271.945865, -7.75, 0.034, 0.0063, 0.777777 },
    [6] = { 271.945865, -7.77, 0.0351, 0.0065, 0.777777 },
    [7] = { 271.945865, -7.8, 0.037, 0.0079, 0.777777 },
    [8] = { 271.945865, -7.8, 0.0348, 0.0048, 0.777777 },
    [9] = { 271.945865, -7.77, 0.0336, 0.0056, 0.777777 },
    [10] = { 271.945865, -7.83, 0.0344, 0.006, 0.777777 },
    [11] = { 271.945865, -7.77, 0.0347, 0.0089, 0.777777 },
    [12] = { 271.945865, -7.741, 0.035, 0.0062, 0.777777 },
    [13] = { 271.945865, -7.85, 0.037, 0.0083, 0.777777 },
    [14] = { 271.945865, -7.77, 0.037, 0.0069, 0.777777 },
    [15] = { 271.945865, -7.83, 0.0357, 0.00485, 0.777777 },
    [16] = { 271.945865, -7.83, 0.0354, 0.0069, 0.777777 },
    [17] = { 271.945865, -7.78, 0.035, 0.001, 0.777777 },
    [18] = { 271.945865, -7.76, 0.035, 0.0044, 0.777777 },
    [19] = { 271.945865, -7.89, 0.0369, 0.00565, 0.777777 },
    [20] = { 271.945865, -7.845, 0.03672, 0.00485, 0.777777 },
    [21] = { 271.945865, -7.89, 0.0357, 0.0058, 0.777777 },
    [22] = { 271.945865, -7.79, 0.0338, 0.0042, 0.777777 },
    [23] = { 271.945865, -7.75, 0.0347, 0.01, 0.777777 },
    [24] = { 271.945865, -7.73, 0.0338, 0.0075, 0.777777 },
    [25] = { 271.945865, -7.72, 0.0348, 0.01035, 0.777777 },
    [26] = { 271.945865, -7.81, 0.0354, 0.0071, 0.777777 },
    [27] = { 271.945865, -7.782, 0.0362, 0.0102, 0.777777 },
    [28] = { 271.945865, -7.88, 0.036, 0.0091, 0.777777 },
    [29] = { 271.945865, -7.8, 0.0355, 0.0065, 0.777777 },
    [30] = { 271.945865, -7.83, 0.0347, 0.0093, 0.777777 },
    [31] = { 271.945865, -7.801, 0.0354, 0.0055, 0.777777 },
    [32] = { 271.945865, -7.82, 0.0352, 0.0094, 0.777777 },
    [33] = { 271.945865, -7.77, 0.035, 0.0083, 0.777777 },
    [34] = { 271.945865, -7.83, 0.035, 0.0097, 0.777777 },
    [35] = { 271.945865, -7.88, 0.036, 0.0079, 0.777777 },
}

-- Mirror the center tile on the server.

mirror = 18

w[0] = w[mirror];
v[0] = v[mirror];
p[0] = p[mirror];

line_screen[0] = line_screen[mirror];

num[0] = 0
map[0] = 0

-------------------------------------------------------------------------------

-- Configure all hosts.

for i = 0, 35 do
    local k = map[i]

    tile[i] = E.add_tile(num[i], w[i][1], w[i][2], w[i][3], w[i][4])

    E.set_tile_viewport(tile[i], v[i][1], v[i][2], v[i][3], v[i][4])
    E.set_tile_position(tile[i], p[i][1], p[i][2], p[i][3], p[i][4],
                        p[i][5], p[i][6], p[i][7], p[i][8], p[i][9])

--  E.set_tile_flags(tile[i], E.tile_flag_local_rot, true);
end

varrier_init()

-------------------------------------------------------------------------------
