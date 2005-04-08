
w = 1600
h = 1200

x = { -2, -1, 0, 1 }
y = { -2.5, -1.5, -0.5, 0.5, 1.5 }
z   = -2

L = { -3200, -1600, 0, 1600 }
B = { -3000, -1800, -600, 600, 1800 }

R = { 0.92, 0, 0 }
U = { 0, 0.92, 0 }

E.add_host("server", 0, 0, 800, 600)
E.add_tile("server", 0, 0, 800, 600, 0, 0, 800, 600, { -2, -2, -2 }, { 4, 0, 0 }, { 0, 4, 0 })

E.add_host("nico1-10",  0, 0, w * 2, h)
E.add_tile("nico1-10",  0, 0, w, h, L[3], B[5], w, h, { x[3], y[5], z }, R, U)
E.add_tile("nico1-10",  w, 0, w, h, L[4], B[5], w, h, { x[4], y[5], z }, R, U)

E.add_host("nico2-10",  0, 0, w * 2, h)
E.add_tile("nico2-10",  0, 0, w, h, L[3], B[4], w, h, { x[3], y[4], z }, R, U)
E.add_tile("nico2-10",  w, 0, w, h, L[4], B[4], w, h, { x[4], y[4], z }, R, U)

E.add_host("nico3-10",  0, 0, w * 2, h)
E.add_tile("nico3-10",  0, 0, w, h, L[3], B[3], w, h, { x[3], y[3], z }, R, U)
E.add_tile("nico3-10",  w, 0, w, h, L[4], B[3], w, h, { x[4], y[3], z }, R, U)

E.add_host("nico4-10",  0, 0, w * 2, h)
E.add_tile("nico4-10",  0, 0, w, h, L[3], B[2], w, h, { x[3], y[2], z }, R, U)
E.add_tile("nico4-10",  w, 0, w, h, L[4], B[2], w, h, { x[4], y[2], z }, R, U)

E.add_host("nico5-10",  0, 0, w * 2, h)
E.add_tile("nico5-10",  0, 0, w, h, L[3], B[1], w, h, { x[3], y[1], z }, R, U)
E.add_tile("nico5-10",  w, 0, w, h, L[4], B[1], w, h, { x[4], y[1], z }, R, U)

E.add_host("nico6-10",  0, 0, w * 2, h)
E.add_tile("nico6-10",  0, 0, w, h, L[1], B[5], w, h, { x[1], y[5], z }, R, U)
E.add_tile("nico6-10",  w, 0, w, h, L[2], B[5], w, h, { x[2], y[5], z }, R, U)

E.add_host("nico7-10",  0, 0, w * 2, h)
E.add_tile("nico7-10",  0, 0, w, h, L[1], B[4], w, h, { x[1], y[4], z }, R, U)
E.add_tile("nico7-10",  w, 0, w, h, L[2], B[4], w, h, { x[2], y[4], z }, R, U)

E.add_host("nico8-10",  0, 0, w * 2, h)
E.add_tile("nico8-10",  0, 0, w, h, L[1], B[3], w, h, { x[1], y[3], z }, R, U)
E.add_tile("nico8-10",  w, 0, w, h, L[2], B[3], w, h, { x[2], y[3], z }, R, U)

E.add_host("nico9-10",  0, 0, w * 2, h)
E.add_tile("nico9-10",  0, 0, w, h, L[1], B[2], w, h, { x[1], y[2], z }, R, U)
E.add_tile("nico9-10",  w, 0, w, h, L[2], B[2], w, h, { x[2], y[2], z }, R, U)

E.add_host("nico10-10", 0, 0, w * 2, h)
E.add_tile("nico10-10", 0, 0, w, h, L[1], B[1], w, h, { x[1], y[1], z }, R, U)
E.add_tile("nico10-10", w, 0, w, h, L[2], B[1], w, h, { x[2], y[1], z }, R, U)

-------------------------------------------------------------------------------
