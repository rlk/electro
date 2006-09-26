tumble = false
scale  = false
turn   = false

zoom  = 0.0001
rot_x = 25.2
rot_y = 0
rot_d = 0
pan_x = 0
pan_y = 0
pan_z = 0

magnitude = 100

X0 = 0
Y0 = 0
Z0 = 0
X1 = 0
Y1 = 0
Z1 = 0
XC = 0
YC = 0
ZC = 0

-------------------------------------------------------------------------------

function do_start()
    X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    XC = (X0 + X1) / 2
    YC = (Y0 + Y1) / 2
    ZC = (Z0 + Z1) / 2

    local star_brush = E.create_brush()
    E.set_brush_frag_prog(star_brush, "../data/star.fp")
    E.set_brush_vert_prog(star_brush, "../data/star.vp")

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    pivot  = E.create_pivot()
    wand   = E.create_pivot()
    galaxy = E.create_galaxy("../data/galaxy_hip.gal", star_brush)

    E.parent_entity(light,  camera)
    E.parent_entity(galaxy, camera)
    E.parent_entity(pivot,  light)
    E.parent_entity(wand,   light)

--  E.parent_entity(E.create_terrain("megr008.raw",  2880,  1440, 90), pivot)
--  E.parent_entity(E.create_terrain("megr016.raw",  5760,  2880, 90), pivot)
--  E.parent_entity(E.create_terrain("megr032.raw", 11520,  5760, 90), pivot)
    E.parent_entity(E.create_terrain("megr032.raw", 11520,  5760, 45), pivot)
--  E.parent_entity(E.create_terrain("megr064.raw", 23040, 11520, 45), pivot)
--  E.parent_entity(E.create_terrain("megr064.raw", 23040, 11520, 90), pivot)
    
    E.set_galaxy_magnitude(galaxy, magnitude)
    E.set_camera_range(camera, 10, 10000)

    E.set_entity_position(light,  1.0, 0.0, 1.0)
    E.set_entity_position(pivot,  XC,  YC,  ZC)
    E.set_entity_position(camera, 0.0, 0.0, 1000.0)
    E.set_entity_rotation(pivot, rot_x, rot_y, 0)
    E.set_entity_scale   (pivot, zoom, zoom, zoom)

    E.set_entity_tracking(wand, 1, E.tracking_mode_local)
    E.set_entity_flags   (wand, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand, E.entity_flag_track_rot, true)

    E.enable_timer(true)
end

function do_timer(dt)
    local joy_x, joy_y = E.get_joystick(0)
    local s = 100

    if joy_x < -0.1 or 0.1 < joy_x then
        E.turn_entity(camera, 0, -joy_x * dt * 90, 0)
    end

    if joy_y < -0.1 or 0.1 < joy_y then
        mov_x, mov_y, mov_z = E.get_entity_z_vector(wand)
        E.move_entity(camera, -mov_x * joy_y * dt * s,
                              -mov_y * joy_y * dt * s,
                              -mov_z * joy_y * dt * s)
    else
        mov_x, mov_y, mov_z = E.get_entity_x_vector(wand)
        E.move_entity(camera, -mov_x * pan_x * dt * s,
                              -mov_y * pan_x * dt * s,
                              -mov_z * pan_x * dt * s)

        mov_x, mov_y, mov_z = E.get_entity_y_vector(wand)
        E.move_entity(camera, -mov_x * pan_y * dt * s,
                              -mov_y * pan_y * dt * s,
                              -mov_z * pan_y * dt * s)

        mov_x, mov_y, mov_z = E.get_entity_z_vector(wand)
        E.move_entity(camera, -mov_x * pan_z * dt * s,
                              -mov_y * pan_z * dt * s,
                              -mov_z * pan_z * dt * s)
    end

    if rot_d < 0 or 0 < rot_d then
        rot_y = rot_y + rot_d * dt * 10
        E.set_entity_rotation(pivot, rot_x, rot_y, 0)
    end

    E.set_entity_position(galaxy, E.get_entity_position(camera));

    return true
end

function do_click(b, s)
    if b == 1 then
        turn = s
    end
    if b == 2 then
        tumble = s
    end
    if b == 3 then
        scale  = s
    end

    return true
end

function do_point(dx, dy)
    if tumble then
--      rot_x = rot_x + 0.25 * dy / zoom
        rot_y = rot_y + 2500 * dx * zoom

--      if rot_x >  90.0 then rot_x =  90 end
--      if rot_x < -90.0 then rot_x = -90 end

        E.set_entity_rotation(pivot, rot_x, rot_y, 0)

        return true
    end

    if turn then
        E.turn_entity(camera, -dy, -dx, 0)

        return true
    end

    if scale then
        zoom = zoom + dy / 500000

        E.set_entity_scale(pivot, zoom, zoom, zoom)

        return true
    end

    return false
end

function do_keyboard(k, s)
    local d = 0.125

    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then
        if k == E.key_return then
            E.set_entity_rotation(pivot,  0.0, 0.0, 0.0)
            E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
            E.set_entity_position(camera, 0.0, 0.0, 5000000.0)
            rot_x = 0
            rot_y = 0
            pan_x = 0
            pan_y = 0
            pan_z = 0
            return true
        end

	if k == E.key_insert then
            rot_d = rot_d + 0.1
	end
	if k == E.key_delete then
            rot_d = rot_d - 0.1
	end

        if not E.get_modifier(E.key_modifier_control) then
            if k == E.key_up       then pan_z = pan_z + 1 end
            if k == E.key_down     then pan_z = pan_z - 1 end
            if k == E.key_pagedown then pan_y = pan_y + 1 end
            if k == E.key_pageup   then pan_y = pan_y - 1 end
            if k == E.key_left     then pan_x = pan_x + 1 end
            if k == E.key_right    then pan_x = pan_x - 1 end
        end
    else
        if not E.get_modifier(E.key_modifier_control) then
            if k == 273 then pan_z = pan_z - 1 end
            if k == 274 then pan_z = pan_z + 1 end
            if k == 281 then pan_y = pan_y - 1 end
            if k == 280 then pan_y = pan_y + 1 end
            if k == 276 then pan_x = pan_x - 1 end
            if k == 275 then pan_x = pan_x + 1 end
        end
    end

    return false
end

-------------------------------------------------------------------------------

do_start()
E.set_background(0, 0, 0, 0, 0, 0.05)