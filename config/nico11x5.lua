host = { }
tile = { }

sw = 800             -- server window size in pixels
sh = 375
pw = 1600            -- inside size in pixels
ph = 1200
iw =   17.000 / 12.0 -- inside size in feet
ih =   12.750 / 12.0
mw =    1.375 / 12.0 -- mullion size in feet
mh =    1.250 / 12.0
ow = iw + mw         -- outside size in feet
oh = ih + mh
kw = pw / iw         -- pixels per foot
kh = ph / ih

x = { -iw / 2 - ow * 5,
      -iw / 2 - ow * 4,
      -iw / 2 - ow * 3,
      -iw / 2 - ow * 2,
      -iw / 2 - ow,
      -iw / 2,
      -iw / 2 + ow,
      -iw / 2 + ow * 2,
      -iw / 2 + ow * 3,
      -iw / 2 + ow * 4,
      -iw / 2 + ow * 5 }

y = { -ih / 2 - oh * 2,
      -ih / 2 - oh,
      -ih / 2,
      -ih / 2 + oh,
      -ih / 2 + oh * 2 }

z = -6

L = { (x[1] - x[1]) * kw,
      (x[2] - x[1]) * kw,
      (x[3] - x[1]) * kw,
      (x[4] - x[1]) * kw,
      (x[5] - x[1]) * kw,
      (x[6] - x[1]) * kw,
      (x[7] - x[1]) * kw,
      (x[8] - x[1]) * kw,
      (x[9] - x[1]) * kw,
      (x[10] - x[1]) * kw,
      (x[11] - x[1]) * kw }

B = { (y[1] - y[1]) * kh,
      (y[2] - y[1]) * kh,
      (y[3] - y[1]) * kh,
      (y[4] - y[1]) * kh,
      (y[5] - y[1]) * kh }

host[0]  = E.add_host("nico",       0, 0, sw, sh)
host[1]  = E.add_host("nico18-10",  0, 0, pw, ph)
host[2]  = E.add_host("nico17-10",  0, 0, pw, ph * 2)
host[3]  = E.add_host("yorda1-10",  0, 0, pw, ph * 2)
host[4]  = E.add_host("yorda2-10",  0, 0, pw * 2, ph)
host[5]  = E.add_host("yorda3-10",  0, 0, pw * 2, ph)
host[6]  = E.add_host("yorda4-10",  0, 0, pw * 2, ph)
host[7]  = E.add_host("yorda5-10",  0, 0, pw * 2, ph)
host[8]  = E.add_host("yorda6-10",  0, 0, pw * 2, ph)
host[9]  = E.add_host("yorda7-10",  0, 0, pw * 2, ph)
host[10] = E.add_host("yorda8-10",  0, 0, pw * 2, ph)
host[11] = E.add_host("yorda9-10",  0, 0, pw * 2, ph)
host[12] = E.add_host("yorda10-10", 0, 0, pw * 2, ph)
host[13] = E.add_host("yorda11-10", 0, 0, pw * 2, ph)
host[14] = E.add_host("yorda12-10", 0, 0, pw * 2, ph)
host[15] = E.add_host("yorda13-10", 0, 0, pw * 2, ph)
host[16] = E.add_host("yorda14-10", 0, 0, pw * 2, ph)
host[17] = E.add_host("yorda15-10", 0, 0, pw * 2, ph)
host[18] = E.add_host("yorda16-10", 0, 0, pw * 2, ph)
host[19] = E.add_host("nico6-10",   0, 0, pw * 2, ph)
host[20] = E.add_host("nico7-10",   0, 0, pw * 2, ph)
host[21] = E.add_host("nico8-10",   0, 0, pw * 2, ph)
host[22] = E.add_host("nico9-10",   0, 0, pw * 2, ph)
host[23] = E.add_host("nico10-10",  0, 0, pw * 2, ph)
host[24] = E.add_host("nico1-10",   0, 0, pw * 2, ph)
host[25] = E.add_host("nico2-10",   0, 0, pw * 2, ph)
host[26] = E.add_host("nico3-10",   0, 0, pw * 2, ph)
host[27] = E.add_host("nico4-10",   0, 0, pw * 2, ph)
host[28] = E.add_host("nico5-10",   0, 0, pw * 2, ph)

E.set_host_flag(host[0], E.host_flag_framed, true)

tile[ 0] = E.add_tile(host[0],       0, 0, sw, sh)
tile[ 1] = E.add_tile(host[1],       0, 0, pw - 1, ph)
tile[ 2] = E.add_tile(host[2],  0, ph - 1, pw - 1, ph)
tile[ 3] = E.add_tile(host[2],       0, 0, pw - 1, ph)
tile[ 4] = E.add_tile(host[3],  0, ph - 1, pw - 1, ph)
tile[ 5] = E.add_tile(host[3],       0, 0, pw - 1, ph)
tile[ 6] = E.add_tile(host[4],       0, 0, pw - 1, ph)
tile[ 7] = E.add_tile(host[4],  pw - 1, 0, pw - 1, ph)
tile[ 8] = E.add_tile(host[5],       0, 0, pw - 1, ph)
tile[ 9] = E.add_tile(host[5],  pw - 1, 0, pw - 1, ph)
tile[10] = E.add_tile(host[6],       0, 0, pw - 1, ph)
tile[11] = E.add_tile(host[6],  pw - 1, 0, pw - 1, ph)
tile[12] = E.add_tile(host[7],       0, 0, pw - 1, ph)
tile[13] = E.add_tile(host[7],  pw - 1, 0, pw - 1, ph)
tile[14] = E.add_tile(host[8],       0, 0, pw - 1, ph)
tile[15] = E.add_tile(host[8],  pw - 1, 0, pw - 1, ph)
tile[16] = E.add_tile(host[9],       0, 0, pw - 1, ph)
tile[17] = E.add_tile(host[9],  pw - 1, 0, pw - 1, ph)
tile[18] = E.add_tile(host[10],      0, 0, pw - 1, ph)
tile[19] = E.add_tile(host[10], pw - 1, 0, pw - 1, ph)
tile[20] = E.add_tile(host[11],      0, 0, pw - 1, ph)
tile[21] = E.add_tile(host[11], pw - 1, 0, pw - 1, ph)
tile[22] = E.add_tile(host[12],      0, 0, pw - 1, ph)
tile[23] = E.add_tile(host[12], pw - 1, 0, pw - 1, ph)
tile[24] = E.add_tile(host[13],      0, 0, pw - 1, ph)
tile[25] = E.add_tile(host[13], pw - 1, 0, pw - 1, ph)
tile[26] = E.add_tile(host[14],      0, 0, pw - 1, ph)
tile[27] = E.add_tile(host[14], pw - 1, 0, pw - 1, ph)
tile[28] = E.add_tile(host[15],      0, 0, pw - 1, ph)
tile[29] = E.add_tile(host[15], pw - 1, 0, pw - 1, ph)
tile[30] = E.add_tile(host[16],      0, 0, pw - 1, ph)
tile[31] = E.add_tile(host[16], pw - 1, 0, pw - 1, ph)
tile[32] = E.add_tile(host[17],      0, 0, pw - 1, ph)
tile[33] = E.add_tile(host[17], pw - 1, 0, pw - 1, ph)
tile[34] = E.add_tile(host[18],      0, 0, pw - 1, ph)
tile[35] = E.add_tile(host[18], pw - 1, 0, pw - 1, ph)
tile[36] = E.add_tile(host[19],      0, 0, pw - 1, ph)
tile[37] = E.add_tile(host[19], pw - 1, 0, pw - 1, ph)
tile[38] = E.add_tile(host[20],      0, 0, pw - 1, ph)
tile[39] = E.add_tile(host[20], pw - 1, 0, pw - 1, ph)
tile[40] = E.add_tile(host[21],      0, 0, pw - 1, ph)
tile[41] = E.add_tile(host[21], pw - 1, 0, pw - 1, ph)
tile[42] = E.add_tile(host[22],      0, 0, pw - 1, ph)
tile[43] = E.add_tile(host[22], pw - 1, 0, pw - 1, ph)
tile[44] = E.add_tile(host[23],      0, 0, pw - 1, ph)
tile[45] = E.add_tile(host[23], pw - 1, 0, pw - 1, ph)
tile[46] = E.add_tile(host[24],      0, 0, pw - 1, ph)
tile[47] = E.add_tile(host[24], pw - 1, 0, pw - 1, ph)
tile[48] = E.add_tile(host[25],      0, 0, pw - 1, ph)
tile[49] = E.add_tile(host[25], pw - 1, 0, pw - 1, ph)
tile[50] = E.add_tile(host[26],      0, 0, pw - 1, ph)
tile[51] = E.add_tile(host[26], pw - 1, 0, pw - 1, ph)
tile[52] = E.add_tile(host[27],      0, 0, pw - 1, ph)
tile[53] = E.add_tile(host[27], pw - 1, 0, pw - 1, ph)
tile[54] = E.add_tile(host[28],      0, 0, pw - 1, ph)
tile[55] = E.add_tile(host[28], pw - 1, 0, pw - 1, ph)
 
E.set_tile_viewport(tile[ 0], L[1], B[1], pw * 11, ph * 5)
E.set_tile_viewport(tile[ 1], L[1], B[5], pw, ph)
E.set_tile_viewport(tile[ 2], L[1], B[4], pw, ph)
E.set_tile_viewport(tile[ 3], L[1], B[3], pw, ph)
E.set_tile_viewport(tile[ 4], L[1], B[2], pw, ph)
E.set_tile_viewport(tile[ 5], L[1], B[1], pw, ph)
E.set_tile_viewport(tile[ 6], L[2], B[5], pw, ph)
E.set_tile_viewport(tile[ 7], L[3], B[5], pw, ph)
E.set_tile_viewport(tile[ 8], L[2], B[4], pw, ph)
E.set_tile_viewport(tile[ 9], L[3], B[4], pw, ph)
E.set_tile_viewport(tile[10], L[2], B[3], pw, ph)
E.set_tile_viewport(tile[11], L[3], B[3], pw, ph)
E.set_tile_viewport(tile[12], L[2], B[2], pw, ph)
E.set_tile_viewport(tile[13], L[3], B[2], pw, ph)
E.set_tile_viewport(tile[14], L[2], B[1], pw, ph)
E.set_tile_viewport(tile[15], L[3], B[1], pw, ph)
E.set_tile_viewport(tile[16], L[4], B[5], pw, ph)
E.set_tile_viewport(tile[17], L[5], B[5], pw, ph)
E.set_tile_viewport(tile[18], L[4], B[4], pw, ph)
E.set_tile_viewport(tile[19], L[5], B[4], pw, ph)
E.set_tile_viewport(tile[20], L[4], B[3], pw, ph)
E.set_tile_viewport(tile[21], L[5], B[3], pw, ph)
E.set_tile_viewport(tile[22], L[4], B[2], pw, ph)
E.set_tile_viewport(tile[23], L[5], B[2], pw, ph)
E.set_tile_viewport(tile[24], L[4], B[1], pw, ph)
E.set_tile_viewport(tile[25], L[5], B[1], pw, ph)
E.set_tile_viewport(tile[26], L[6], B[5], pw, ph)
E.set_tile_viewport(tile[27], L[7], B[5], pw, ph)
E.set_tile_viewport(tile[28], L[6], B[4], pw, ph)
E.set_tile_viewport(tile[29], L[7], B[4], pw, ph)
E.set_tile_viewport(tile[30], L[6], B[3], pw, ph)
E.set_tile_viewport(tile[31], L[7], B[3], pw, ph)
E.set_tile_viewport(tile[32], L[6], B[2], pw, ph)
E.set_tile_viewport(tile[33], L[7], B[2], pw, ph)
E.set_tile_viewport(tile[34], L[6], B[1], pw, ph)
E.set_tile_viewport(tile[35], L[7], B[1], pw, ph)
E.set_tile_viewport(tile[36], L[8], B[5], pw, ph)
E.set_tile_viewport(tile[37], L[9], B[5], pw, ph)
E.set_tile_viewport(tile[38], L[8], B[4], pw, ph)
E.set_tile_viewport(tile[39], L[9], B[4], pw, ph)
E.set_tile_viewport(tile[40], L[8], B[3], pw, ph)
E.set_tile_viewport(tile[41], L[9], B[3], pw, ph)
E.set_tile_viewport(tile[42], L[8], B[2], pw, ph)
E.set_tile_viewport(tile[43], L[9], B[2], pw, ph)
E.set_tile_viewport(tile[44], L[8], B[1], pw, ph)
E.set_tile_viewport(tile[45], L[9], B[1], pw, ph)
E.set_tile_viewport(tile[46], L[10], B[5], pw, ph)
E.set_tile_viewport(tile[47], L[11], B[5], pw, ph)
E.set_tile_viewport(tile[48], L[10], B[4], pw, ph)
E.set_tile_viewport(tile[49], L[11], B[4], pw, ph)
E.set_tile_viewport(tile[50], L[10], B[3], pw, ph)
E.set_tile_viewport(tile[51], L[11], B[3], pw, ph)
E.set_tile_viewport(tile[52], L[10], B[2], pw, ph)
E.set_tile_viewport(tile[53], L[11], B[2], pw, ph)
E.set_tile_viewport(tile[54], L[10], B[1], pw, ph)
E.set_tile_viewport(tile[55], L[11], B[1], pw, ph)

E.set_tile_position(tile[ 0], x[1], y[1], z, iw * 11, 0, 0, 0, ih * 5, 0)
E.set_tile_position(tile[ 1], x[1], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 2], x[1], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 3], x[1], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 4], x[1], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 5], x[1], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 6], x[2], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 7], x[3], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 8], x[2], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[ 9], x[3], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[10], x[2], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[11], x[3], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[12], x[2], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[13], x[3], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[14], x[2], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[15], x[3], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[16], x[4], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[17], x[5], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[18], x[4], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[19], x[5], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[20], x[4], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[21], x[5], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[22], x[4], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[23], x[5], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[24], x[4], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[25], x[5], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[26], x[6], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[27], x[7], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[28], x[6], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[29], x[7], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[30], x[6], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[31], x[7], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[32], x[6], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[33], x[7], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[34], x[6], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[35], x[7], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[36], x[8], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[37], x[9], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[38], x[8], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[39], x[9], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[40], x[8], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[41], x[9], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[42], x[8], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[43], x[9], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[44], x[8], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[45], x[9], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[46], x[10], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[47], x[11], y[5], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[48], x[10], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[49], x[11], y[4], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[50], x[10], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[51], x[11], y[3], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[52], x[10], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[53], x[11], y[2], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[54], x[10], y[1], z, iw, 0, 0, 0, ih, 0)
E.set_tile_position(tile[55], x[11], y[1], z, iw, 0, 0, 0, ih, 0)

-------------------------------------------------------------------------------
