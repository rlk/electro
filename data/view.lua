
-------------------------------------------------------------------------------

pnt_x  = 0.0
pnt_y  = 0.0

rot_x  = 0.0
rot_y  = 0.0

persp  = nil
ortho  = nil
obj1   = nil
obj2   = nil
light  = nil
point  = nil
btn    = { }

-------------------------------------------------------------------------------

function init_2D()
    ortho = camera_create(1)
    point = sprite_create("head.png")

    entity_parent(point, ortho)
    entity_scale(point, 0.5, 0.5, 0.5)
end

function init_3D()
    persp = camera_create(2)

    light = light_create(2)
    pivot = pivot_create()
    obj1  = object_create("ball.obj")
    obj2  = object_create("ball.obj")

    entity_parent(light, persp)
    entity_parent(pivot, light)
    entity_parent(obj1,  pivot)
    entity_parent(obj2,  pivot)

    camera_zoom(persp,  0.001)
    camera_dist(persp, 10.000)

    entity_position(light, 1, 1, 1)
    entity_position(obj1, -1, 0, 0)
    entity_position(obj2,  1, 0, 0)
end

function do_start()
    init_2D()
    init_3D()

    enable_idle(true)

    return true
end

function do_click(b, s)
   btn[b] = s
end

function do_timer(dt)
    rot_y = rot_y + dt * 90
    if (pivot) then entity_rotation(pivot, rot_x, rot_y, 0) end
    print("foo")
    return true
end

function do_point(dx, dy)
    pnt_x = pnt_x + dx
    pnt_y = pnt_y - dy

    if (point) then entity_position(point, pnt_x, pnt_y, 0) end

    if btn[1] then
        rot_x = rot_x + dy
        rot_y = rot_y + dx

        if (pivot) then entity_rotation(pivot, rot_x, rot_y, 0) end
    end

    return true
end

-------------------------------------------------------------------------------
