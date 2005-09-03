local path = E.path

video_K = 2827
video_w =  640
video_h =  960
video_m =  480
video_b =   12

off_x = 0
off_z = 0

-------------------------------------------------------------------------------

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s and k == E.key_left  then off_x = off_x - 1 / 32 end
    if s and k == E.key_right then off_x = off_x + 1 / 32 end
    if s and k == E.key_up    then off_z = off_z - 1 / 32 end
    if s and k == E.key_down  then off_z = off_z + 1 / 32 end

    if s and k == E.key_F9 then
        E.set_sprite_range(spriteL, 0, video_w, video_m, 0)
        E.set_sprite_range(spriteR, 0, video_w, video_h, video_m)
    end
    if s and k == E.key_F10 then
        E.set_sprite_range(spriteL, video_w, 0, video_m, 0)
        E.set_sprite_range(spriteR, video_w, 0, video_h, video_m)
    end
    if s and key == E.key_F12 then
        E.exec(path.."../demo.lua")
    end

    E.set_entity_position(spriteL, pos_x + off_x, pos_y, pos_z + off_z)
    E.set_entity_position(spriteR, pos_x - off_x, pos_y, pos_z + off_z)

    return false
end

function do_timer(dt)
    return true
end

function do_start()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    camera  = E.create_camera(E.camera_type_perspective)
    videoL  = E.create_image(video_K, video_w, video_h, video_b)
    videoR  = E.create_image(video_K, video_w, video_h, video_b)
    brushL  = E.create_brush()
    brushR  = E.create_brush()
    spriteL = E.create_sprite(brushL)
    spriteR = E.create_sprite(brushR)

    E.parent_entity(spriteL, camera)
    E.parent_entity(spriteR, camera)

    E.set_entity_flags(spriteL, E.entity_flag_left_eye,  true)
    E.set_entity_flags(spriteR, E.entity_flag_right_eye, true)

    E.set_brush_image(brushL, videoL)
    E.set_brush_image(brushR, videoR)
    E.set_brush_flags(brushL, E.brush_flag_unlit, true)
    E.set_brush_flags(brushR, E.brush_flag_unlit, true)

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(spriteL)
    local s = math.min((X1 - X0) / (x1 - x0), 2 * (Y1 - Y0) / (y1 - y0))

    E.set_sprite_range(spriteL, 0, video_w, video_m, 0)
    E.set_sprite_range(spriteR, 0, video_w, video_h, video_m)

    pos_x = (X0 + X1) / 2
    pos_y = (Y0 + Y1) / 2
    pos_z = (Z0 + Z1) / 2

    E.set_entity_scale(spriteL, s, s / 2, s)
    E.set_entity_scale(spriteR, s, s / 2, s)
    E.set_entity_position(spriteL, pos_x + off_x, pos_y, pos_z + off_z)
    E.set_entity_position(spriteR, pos_x - off_x, pos_y, pos_z + off_z)

    E.enable_timer(true)
    E.set_background(0, 0, 0)
end

-------------------------------------------------------------------------------

do_start()
