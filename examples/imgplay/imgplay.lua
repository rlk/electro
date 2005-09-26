frame0 =   0
framei =   0
framen =  42

video_w =  2560
video_h =   800
video_m =  1280

image = nil
brush = nil
total = 0

off_x = 3 / 32
off_z = 0

-------------------------------------------------------------------------------

pathname = "/disk2/varrier/ncmir/final-left-right"

function set_image(i)
    E.set_brush_image(brush, nil)
    
    if image then
        E.delete_image(image)
    end

    image = E.create_image(string.format("%s/%04d.png", pathname, i))
    E.set_brush_image(brush, image)
end

-------------------------------------------------------------------------------

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == E.key_left  then off_x = off_x - 1 / 32 end
        if k == E.key_right then off_x = off_x + 1 / 32 end
        if k == E.key_up    then off_z = off_z - 1 / 32 end
        if k == E.key_down  then off_z = off_z + 1 / 32 end

        if k == E.key_F9 then
            E.set_sprite_range(spriteL,       0, video_m, 0, video_h)
            E.set_sprite_range(spriteR, video_m, video_w, 0, video_h)
        end
        if k == E.key_F10 then
            E.set_sprite_range(spriteL, video_m,       0, 0, video_h)
            E.set_sprite_range(spriteR, video_w, video_m, 0, video_h)
        end
        if k == E.key_F12 then
            E.nuke()
            E.chdir("..")
            dofile("demo.lua")
            return true
        end

        E.set_entity_position(spriteL, pos_x + off_x, pos_y, pos_z + off_z)
        E.set_entity_position(spriteR, pos_x - off_x, pos_y, pos_z + off_z)
        
        return true
    end
    return false
end

function do_frame()
    framei = framei + 1

    if framei > framen then
        framei = frame0
    end

    set_image(framei)
end

function do_timer(dt)
    total = total + dt

    if (total > 0.1) then
        total = 0
        return true
    else
        return false
    end
end

function do_start()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    brush = E.create_brush()
    set_image(frame0)

    camera  = E.create_camera(E.camera_type_perspective)
    spriteL = E.create_sprite(brush)
    spriteR = E.create_sprite(brush)

    E.parent_entity(spriteL, camera)
    E.parent_entity(spriteR, camera)

    E.set_entity_flags(spriteL, E.entity_flag_left_eye,  true)
    E.set_entity_flags(spriteR, E.entity_flag_right_eye, true)

    E.set_brush_image(brush, image)
    E.set_brush_flags(brush, E.brush_flag_unlit, true)

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(spriteL)
    local s = math.min(2 * (X1 - X0) / (x1 - x0), (Y1 - Y0) / (y1 - y0))

    E.set_sprite_range(spriteL,       0, video_m, 0, video_h)
    E.set_sprite_range(spriteR, video_m, video_w, 0, video_h)

--    E.set_sprite_range(spriteL, 0, video_w, video_h, 0)
--    E.set_sprite_range(spriteR, 0, video_w, video_h, 0)

    pos_x = (X0 + X1) / 2
    pos_y = (Y0 + Y1) / 2
    pos_z = (Z0 + Z1) / 2

    E.set_entity_scale(spriteL, s / 2, s, s)
    E.set_entity_scale(spriteR, s / 2, s, s)
    E.set_entity_position(spriteL, pos_x + off_x, pos_y, pos_z + off_z)
    E.set_entity_position(spriteR, pos_x - off_x, pos_y, pos_z + off_z)

    E.enable_timer(true)
    E.set_background(0, 0, 0)
end

-------------------------------------------------------------------------------

do_start()
