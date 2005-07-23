-------------------------------------------------------------------------------
-- This is an example use of the object creation API.  It creates and returns
-- a cube textured using the named image file.

function make_side(O, m, p1, p2, p3, p4, n)
    local v1 = E.create_vert(O, p1[1], p1[2], p1[3], n[1], n[2], n[3], 0, 0)
    local v2 = E.create_vert(O, p2[1], p2[2], p2[3], n[1], n[2], n[3], 1, 0)
    local v3 = E.create_vert(O, p3[1], p3[2], p3[3], n[1], n[2], n[3], 1, 1)
    local v4 = E.create_vert(O, p4[1], p4[2], p4[3], n[1], n[2], n[3], 0, 1)

    E.create_face(O, m, v1, v2, v3)
    E.create_face(O, m, v3, v4, v1)
end

function make_cube(file)
    local v000 = { -1, -1, -1 }
    local v001 = { -1, -1,  1 }
    local v010 = { -1,  1, -1 }
    local v011 = { -1,  1,  1 }
    local v100 = {  1, -1, -1 }
    local v101 = {  1, -1,  1 }
    local v110 = {  1,  1, -1 }
    local v111 = {  1,  1,  1 }

    local O = E.create_object()
    local i = E.create_image(file)
    local b = E.create_brush()
    local m = E.create_mesh(O, b)

    E.set_brush_image(b, i)

    make_side(O, m, v100, v000, v010, v110, {  0,  0, -1 })
    make_side(O, m, v001, v101, v111, v011, {  0,  0,  1 })
    make_side(O, m, v000, v100, v101, v001, {  0, -1,  0 })
    make_side(O, m, v011, v111, v110, v010, {  0,  1,  0 })
    make_side(O, m, v000, v001, v011, v010, { -1,  0,  0 })
    make_side(O, m, v101, v100, v110, v111, {  1,  0,  0 })

    return O
end

-------------------------------------------------------------------------------
