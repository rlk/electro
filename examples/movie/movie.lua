stereo = true
port   = 2827

image = nil
brush = nil
total = 0

off_x = 0
off_y = 0
off_z = 0

-------------------------------------------------------------------------------
function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == E.key_left      then off_x = off_x - 1 / 128 end
        if k == E.key_right     then off_x = off_x + 1 / 128 end
        if k == E.key_up        then off_y = off_y - 1 / 128 end
        if k == E.key_down      then off_y = off_y + 1 / 128 end
        if k == E.key_pageup    then off_z = off_z - 1 / 32  end
        if k == E.key_pagedown  then off_z = off_z + 1 / 32  end

        if k == E.key_F12 then
            E.nuke()
            E.chdir("..")
            dofile("demo.lua")
            return true
        end

        E.set_entity_position(spriteL, pos_x + off_x,
                                       pos_y + off_y,
                                       pos_z + off_z)
        E.set_entity_position(spriteR, pos_x - off_x,
                                       pos_y - off_y,
                                       pos_z + off_z)
        
        return true
    end
    return false
end

function do_timer(dt)
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()
    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(spriteL)

    local w = (x1 - x0)
    local h = (y1 - y0)
    local m = h / 2
    local s

    if stereo then
--        s = math.min((X1 - X0) / (x1 - x0), (Y1 - Y0) / (y1 - y0))
        s = math.min((X1 - X0) / (x1 - x0), (Y1 - Y0) / (y1 - y0))
        E.set_sprite_range(spriteL, 0, w, m, 0)
        E.set_sprite_range(spriteR, 0, w, h, m)
        E.set_entity_scale(spriteL, 2 * s, s, s)
        E.set_entity_scale(spriteR, 2 * s, s, s)
--        E.set_entity_scale(spriteL, s / 2, s, s)
--        E.set_entity_scale(spriteR, s / 2, s, s)
    else
        s = math.min((X1 - X0) / (x1 - x0), (Y1 - Y0) / (y1 - y0))
        E.set_sprite_range(spriteL, 0, w, 0, h)
        E.set_sprite_range(spriteR, 0, w, 0, h)
        E.set_entity_scale(spriteL, s, s, s)
        E.set_entity_scale(spriteR, s, s, s)
    end

    return true
end

function do_start()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    brush = E.create_brush()
    image = E.create_image(port)

    camera  = E.create_camera(E.camera_type_perspective)
    spriteL = E.create_sprite(brush)
    spriteR = E.create_sprite(brush)

    E.parent_entity(spriteL, camera)
    E.parent_entity(spriteR, camera)

    E.set_entity_flags(spriteL, E.entity_flag_left_eye,  true)
    E.set_entity_flags(spriteR, E.entity_flag_right_eye, true)

    E.set_brush_image(brush, image)
    E.set_brush_flags(brush, E.brush_flag_unlit, true)

    pos_x = (X0 + X1) / 2
    pos_y = (Y0 + Y1) / 2
    pos_z = (Z0 + Z1) / 2

    E.set_entity_position(spriteL, pos_x + off_x, pos_y, pos_z + off_z)
    E.set_entity_position(spriteR, pos_x - off_x, pos_y, pos_z + off_z)

    E.enable_timer(true)
--  E.set_background(0.2, 0.2, 0.2)
    E.set_background(0, 0, 0)
end

-------------------------------------------------------------------------------

do_start()
