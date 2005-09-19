that_K = 3001
that_w =  640
that_h =  960
that_m =  480

this_K = 3002
this_w =  640
this_h =  960
this_m =  480

off_x = 0
off_z = 0

inset_s = 5

-------------------------------------------------------------------------------

function set_positions()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()
    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(that_sprite_L)

    local s = math.min((X1 - X0) / (x1 - x0), 2 * (Y1 - Y0) / (y1 - y0))

    pos_x = (X0 + X1) / 2
    pos_y = (Y0 + Y1) / 2
    pos_z = (Z0 + Z1) / 2

    E.set_entity_scale   (that_sprite_L, s, s / 2, s)
    E.set_entity_scale   (that_sprite_R, s, s / 2, s)
    E.set_entity_position(that_sprite_L, pos_x + off_x, pos_y, pos_z + off_z)
    E.set_entity_position(that_sprite_R, pos_x - off_x, pos_y, pos_z + off_z)

    pos_x = pos_x + (X1 - X0) / 3
    pos_y = pos_y - (Y1 - Y0) / 3
    pos_z = pos_z + 0.1
    s = s / inset_s

    E.set_entity_scale   (this_sprite_L, s, s / 2, s)
    E.set_entity_scale   (this_sprite_R, s, s / 2, s)
    E.set_entity_position(this_sprite_L, pos_x + off_x / inset_s, pos_y,
                                         pos_z + off_z)
    E.set_entity_position(this_sprite_R, pos_x - off_x / inset_s, pos_y,
                                         pos_z + off_z)
end

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
            E.set_sprite_range(spriteL, 0, that_w, that_m, 0)
            E.set_sprite_range(spriteR, 0, that_w, that_h, that_m)
        end
        if k == E.key_F10 then
            E.set_sprite_range(spriteL, that_w, 0, that_m, 0)
            E.set_sprite_range(spriteR, that_w, 0, that_h, that_m)
        end
        if k == E.key_F12 then
            E.nuke()
            E.chdir("..")
            dofile("demo.lua")
            return true
        end

        set_positions()
        
        return true
    end
    return false
end

function do_timer(dt)
    return true
end

function do_start()
    camera  = E.create_camera(E.camera_type_perspective)
    
--  E.set_camera_stereo(camera, E.stereo_mode_red_blue, 0, 0, 0, 0, 0, 0)

    -- Create images and brush for local and remote video.

    that_image = E.create_image(that_K)
    this_image = E.create_image(this_K)
    that_brush = E.create_brush()
    this_brush = E.create_brush()

    E.set_brush_image(that_brush, that_image)
    E.set_brush_image(this_brush, this_image)
    E.set_brush_flags(that_brush, E.brush_flag_unlit, true)
    E.set_brush_flags(this_brush, E.brush_flag_unlit, true)

    -- Create sprites for locale and remote video for the left and right eyes.

    that_sprite_L = E.create_sprite(that_brush)
    that_sprite_R = E.create_sprite(that_brush)
    this_sprite_L = E.create_sprite(this_brush)
    this_sprite_R = E.create_sprite(this_brush)

    E.parent_entity(that_sprite_L, camera)
    E.parent_entity(that_sprite_R, camera)
    E.parent_entity(this_sprite_L, camera)
    E.parent_entity(this_sprite_R, camera)

    E.set_entity_flags(that_sprite_L, E.entity_flag_left_eye,  true)
    E.set_entity_flags(that_sprite_R, E.entity_flag_right_eye, true)
    E.set_entity_flags(this_sprite_L, E.entity_flag_left_eye,  true)
    E.set_entity_flags(this_sprite_R, E.entity_flag_right_eye, true)

    E.set_sprite_range(that_sprite_L, 0, that_w, that_m, 0)
    E.set_sprite_range(that_sprite_R, 0, that_w, that_h, that_m)
    E.set_sprite_range(this_sprite_L, 0, this_w, this_m, 0)
    E.set_sprite_range(this_sprite_R, 0, this_w, this_h, this_m)

    set_positions()

    E.enable_timer(true)
    E.set_background(0, 0, 0)
end

-------------------------------------------------------------------------------

do_start()
