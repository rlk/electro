
video_K = 2827
video_w = 1280
video_h =  480
video_b =    3

-------------------------------------------------------------------------------

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s and k == E.key_F7 then
        E.set_sprite_range(sprite, 0, video_w, video_h, 0)
    end
    if s and k == E.key_F8 then
        E.set_sprite_range(sprite, video_w, 0, video_h, 0)
    end

    return false
end

function do_start()
--  local X, Y, W, H = E.get_display_union()
    local X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    video  = E.create_image(video_K, video_w, video_h, video_b)
    brush  = E.create_brush()
--  camera = E.create_camera(E.camera_type_orthogonal)
    camera = E.create_camera(E.camera_type_perspective)
    sprite = E.create_sprite(brush)

    E.parent_entity(sprite, camera)
    E.set_brush_image(brush, video)
    E.set_brush_flags(brush, E.brush_flag_unlit, true)

    E.set_entity_flags(sprite, E.entity_flag_stereo, true)

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(sprite)
--  local s = math.min(W / (x1 - x0), H / (y1 - y0))
    local s = math.min((X1 - X0) / (x1 - x0), (Y1 - Y0) / (y1 - y0))

    E.set_sprite_range(sprite, 0, video_w, video_h, 0)

    E.set_entity_scale(sprite, s, s, s)
    E.set_entity_position(sprite, (X0 + X1) / 2,
                                  (Y0 + Y1) / 2,
                                  (Z0 + Z1) / 2)

    E.enable_timer(true)
    E.set_background(0, 0, 0)
end

-------------------------------------------------------------------------------

do_start()
