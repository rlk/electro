
-------------------------------------------------------------------------------

w  = 1280
h  = 1024
dx = w + 384  -- Tile width including mullion
dy = h + 384  -- Tile height including mullion
x  = dx * 5 / 2
y  = dy * 3 / 2

add_tile("yorda1-10",   x,          y,          0, 0, w, h)
add_tile("yorda2-10",   x,          y + dy,     0, 0, w, h)
add_tile("yorda3-10",   x,          y + dy * 2, 0, 0, w, h)
add_tile("yorda4-10",   x + dx,     y,          0, 0, w, h)
add_tile("yorda5-10",   x + dx,     y + dy,     0, 0, w, h)
add_tile("yorda6-10",   x + dx,     y + dy * 2, 0, 0, w, h)
add_tile("yorda7-10",   x + dx * 2, y,          0, 0, w, h)
add_tile("yorda8-10",   x + dx * 2, y + dy,     0, 0, w, h)
add_tile("yorda9-10",   x + dx * 2, y + dy * 2, 0, 0, w, h)
add_tile("yorda10-10",  x + dx * 4, y,          0, 0, w, h)
add_tile("yorda11-10",  x + dx * 4, y + dy,     0, 0, w, h)
add_tile("yorda12-10",  x + dx * 4, y + dy * 2, 0, 0, w, h)
add_tile("yorda13-10",  x + dx * 5, y,          0, 0, w, h)
add_tile("yorda14-10",  x + dx * 5, y + dy,     0, 0, w, h)
add_tile("yorda15-10",  x + dx * 5, y + dy * 2, 0, 0, w, h)

-------------------------------------------------------------------------------

