eye_x = 0
eye_y = 0
eye_z = 0

host = { }
tile = { }

hsize = 800
vsize = 600
gap   = 100

-- 4 hosts: 1 server, 3 client

host[1]  = E.add_host("default",           0,           0, hsize, vsize)
host[2]  = E.add_host("default", hsize + gap,           0, hsize, vsize)
host[3]  = E.add_host("default",           0, vsize + gap, hsize, vsize)
host[4]  = E.add_host("default", hsize + gap, vsize + gap, hsize, vsize)

-- Tile host numbers.

num = {
   host[1], host[2],
   host[3], host[4]
}

-- Tile sub-windows

w = {
   {      0,    0, hsize, vsize }, {      0,    0, hsize, vsize },
   {      0,    0, hsize, vsize }, {      0,    0, hsize, vsize },
}

-- Tile viewports.

v = {
   {     0, vsize, hsize, vsize }, { hsize, vsize, hsize, vsize },
   {     0,     0, hsize, vsize }, { hsize,     0, hsize, vsize },
}

-- Tile screen locations.

p = {
    { -1.333,  0.000, -2.000, 1.333, 0.000, 0.000, 0.000, 1.000, 0.000 },
    {  0.000,  0.000, -2.000, 1.333, 0.000, 0.000, 0.000, 1.000, 0.000 },
    { -1.333, -1.000, -2.000, 1.333, 0.000, 0.000, 0.000, 1.000, 0.000 },
    {  0.000, -1.000, -2.000, 1.333, 0.000, 0.000, 0.000, 1.000, 0.000 },
}


-- Configure all hosts.

for i = 1, 4 do

    tile[i] = E.add_tile(num[i], w[i][1], w[i][2], w[i][3], w[i][4])

    E.set_tile_viewport(tile[i], v[i][1], v[i][2], v[i][3], v[i][4])
    E.set_tile_position(tile[i], p[i][1], p[i][2], p[i][3], p[i][4],
                        p[i][5], p[i][6], p[i][7], p[i][8], p[i][9])
end

-------------------------------------------------------------------------------
