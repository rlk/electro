
-------------------------------------------------------------------------------

pos_x  =    0.0
pos_y  =    0.0
pos_z  =    0.0
rot_x  =    0.0
rot_y  =    0.0
dist   =    0.0

camera = nil
sprite = nil

-------------------------------------------------------------------------------

function do_start()
    camera = camera_create(1)
    sprite = sprite_create("head.png")

    entity_parent(sprite, camera)

    return true
end

function do_keyboard(k, s)
    print(k)
    return true
end

-------------------------------------------------------------------------------
