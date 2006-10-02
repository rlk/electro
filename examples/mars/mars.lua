-- 32-degree-per-pixel data

--[[
DATA_S = "/data2/evl/rlk/megr032.raw"
DATA_W = 11520
DATA_H = 5760
DATA_N = 45
]]--

-- 64-degree-per-pixel data

DATA_S = "/data2/evl/rlk/megr064.raw"
DATA_W = 23040
DATA_H = 11520
DATA_N = 45

-- 128-degree-per-pixel data
--[[
DATA_S = "/data2/evl/rlk/megr128.raw"
DATA_W = 46080
DATA_H = 23040
DATA_N = 45
]]--

-------------------------------------------------------------------------------

tumble = false
scale  = false
turn   = false

zoom  = 0.0001
rot_x = 25.2
rot_y = 180
rot_d = 0
pan_x = 0
pan_y = 0
pan_z = 0

jx = 0
jy = 0
jz = 0
jw = 0

init_z = 1000

magnitude = 500

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

function speed()
    local cx, cy, cz = E.get_entity_position(camera)
    return (math.sqrt(cx * cx + cy * cy + cz * cz) - 300) / 300
end

fly = false

fly_px = 0
fly_py = 0
fly_pz = 0

fly_xx = 0
fly_xy = 0
fly_xz = 0

fly_yx = 0
fly_yy = 0
fly_yz = 0

fly_zx = 0
fly_zy = 0
fly_zz = 0

function fly_begin()
    fly_px, fly_py, fly_pz = E.get_entity_position(wand)
    fly_xx, fly_xy, fly_xz = E.get_entity_x_vector(wand)
    fly_yx, fly_yy, fly_yz = E.get_entity_y_vector(wand)
    fly_zx, fly_zy, fly_zz = E.get_entity_z_vector(wand)

    fly = true
end

function fly_step(dt)
    if fly then
        local px, py, pz = E.get_entity_position(wand)
        local xx, xy, xz = E.get_entity_x_vector(wand)
        local yx, yy, yz = E.get_entity_y_vector(wand)
        local zx, zy, zz = E.get_entity_z_vector(wand)
        
        local tspeed = 150.0 * speed()
        local rspeed =  50.0

        local dx = (px - fly_px) * dt * tspeed;
        local dy = (py - fly_py) * dt * tspeed;
        local dz = (pz - fly_pz) * dt * tspeed;

        local vx = fly_zx - zx
        local vy = fly_zy - zy
        local vz = fly_zz - zz

        local wx = fly_yx - yx
        local wy = fly_yy - yy
        local wz = fly_yz - yz

        dX = (vx * fly_xx + vy * fly_xy + vz * fly_xz) * dt * rspeed
        dY = (vx * fly_yx + vy * fly_yy + vz * fly_yz) * dt * rspeed
        dZ = (wx * fly_xx + wy * fly_xy + wz * fly_xz) * dt * rspeed

        E.move_entity(camera, dx, dy, dz)
        E.turn_entity(camera, dX, dY, dZ)
    end
end

function fly_end()
    fly = false
end

-------------------------------------------------------------------------------

function do_start()
    X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    XC = (X0 + X1) / 2
    YC = (Y0 + Y1) / 2
    ZC = (Z0 + Z1) / 2

    local star_brush = E.create_brush()
    E.set_brush_frag_prog(star_brush, "star.fp")
    E.set_brush_vert_prog(star_brush, "star.vp")

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_directional)
    back   = E.create_light(E.light_type_directional)
    pivot  = E.create_pivot()
    wand   = E.create_pivot()
    head   = E.create_pivot()
    galaxy = E.create_galaxy("../data/galaxy_hip.gal", star_brush)

    E.parent_entity(light,  camera)
    E.parent_entity(galaxy, camera)
    E.parent_entity(back,   light)
    E.parent_entity(pivot,  back)
    E.parent_entity(wand,   light)

    E.parent_entity(E.create_terrain(DATA_S, DATA_W, DATA_H, DATA_N), pivot)
    
    E.set_galaxy_magnitude(galaxy, magnitude)
    E.set_camera_range(camera, 1, 10000)

    E.set_entity_position(light,  1.0,  0.0,  1.0)
    E.set_entity_position(back,  -1.0,  0.0, -1.0)
    E.set_entity_position(pivot,  XC, YC, ZC)
    E.set_entity_position(camera, 0.0, 0.0, init_z)
    E.set_entity_rotation(pivot, rot_x, rot_y, 0)
    E.set_entity_scale   (pivot, zoom, zoom, zoom)

    E.set_light_color(back, 0.2, 0.3, 0.5)

    E.set_entity_flags(galaxy, E.entity_flag_ballboard, true)
--  E.set_entity_flags(galaxy, E.entity_flag_billboard, true)

    E.set_entity_tracking(wand, 1, E.tracking_mode_local)
    E.set_entity_flags   (wand, E.entity_flag_track_pos, true)
    E.set_entity_flags   (wand, E.entity_flag_track_rot, true)

    E.set_entity_tracking(head, 0, E.tracking_mode_local)
    E.set_entity_flags   (head, E.entity_flag_track_pos, true)

    E.enable_timer(true)
end

function do_joystick(d, b, s)
    if s then
        fly_begin()
    else
        fly_end()
    end
end

function do_timer(dt)
    local joy_x, joy_y = E.get_joystick(0, 0, 1)
    local joy_z, joy_w = E.get_joystick(0, 3, 4)

    local s = 100 * speed()
    local k = 0.0
    local n = 32

    local dr =  90 * dt
    local dp = 100 * dt * speed()

    fly_step(dt)

    jx = ((n - 1) * jx + joy_x) / n
    jy = ((n - 1) * jy + joy_y) / n
    jz = ((n - 1) * jz + joy_z) / n
    jw = ((n - 1) * jw + joy_w) / n

    E.turn_entity(camera, 0, -jx * dr, 0)
    E.move_entity(camera, 0, 0,  jy * dp)
    E.turn_entity(camera, 0, 0, -jz * dr)
    E.turn_entity(camera,  jw * dr, 0, 0)


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

    if rot_d < 0 or 0 < rot_d then
        rot_y = rot_y + rot_d * dt * 10
        E.set_entity_rotation(pivot, rot_x, rot_y, 0)
    end


    local cx, cy, cz = E.get_entity_position(camera)
    local d = math.sqrt(cx * cx + cy * cy + cz * cz)
    
    E.set_camera_range(camera, 0.1, d + 10000)

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
        rot_y = rot_y + 2500 * dx * zoom
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
            rot_x = 25.2
            rot_y = 180
            pan_x = 0
            pan_y = 0
            pan_z = 0
            E.set_entity_rotation(pivot, rot_x, rot_y, 0.0)
            E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
            E.set_entity_position(camera, 0.0, 0.0, init_z)
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
