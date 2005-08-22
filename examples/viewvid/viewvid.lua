
video_K = 2827
video_w = 1280
video_h =  480
video_b =    3

-------------------------------------------------------------------------------

function do_start()
    local X, Y, W, H = E.get_display_union()

    video  = E.create_image(video_K, video_w, video_h, video_b)
    brush  = E.create_brush()
    camera = E.create_camera(E.camera_type_orthogonal)
    sprite = E.create_sprite(brush)

    E.parent_entity(sprite, camera)
    E.set_brush_image(brush, video)
    E.set_brush_flags(brush, E.brush_flag_unlit, true)

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(sprite)
    local s = math.min(W / (x1 - x0), H / (y1 - y0))

    E.set_sprite_range(sprite, 0, video_w, video_h, 0)

    E.set_entity_scale(sprite, s, s, s)
    E.set_entity_position(sprite, W / 2, H / 2, 0)

    E.enable_timer(true)
    E.set_background(0, 0, 0)
end

-------------------------------------------------------------------------------

do_start()