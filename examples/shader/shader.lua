tumble = false
scale  = false

zoom  = 1
rot_x = 0
rot_y = 0

X0 = 0
Y0 = 0
Z0 = 0
X1 = 0
Y1 = 0
Z1 = 0
XC = 0
YC = 0
ZC = 0

function add_object(s)
    local object = E.create_object(s)
    local brush = E.get_mesh(object, 0);
    local image = E.create_image("../data/crate_norm.png")

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(object)
    local s = 0.5 * (X0 - X1) / (x0 - x1)

    E.parent_entity(object, pivot)
    E.set_entity_scale(object, s, s, s)

    E.set_brush_image(brush, image, 1)

    E.set_brush_uniform_sampler(brush, "colormap", 0)
    E.set_brush_uniform_sampler(brush, "normalmap", 1)
    
    E.set_brush_vert_shader(brush, "vert.glsl")
    E.set_brush_frag_shader(brush, "frag.glsl")
end

function do_start()
    X0, Y0, Z0, X1, Y1, Z1 = E.get_display_bound()

    XC = (X0 + X1) / 2
    YC = (Y0 + Y1) / 2
    ZC = (Z0 + Z1) / 2

    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    pivot  = E.create_pivot()

    E.parent_entity(light, camera)
    E.parent_entity(pivot, light)

    E.set_entity_position(light, XC, YC + 4, ZC + 4)
    E.set_entity_position(pivot, XC, YC, ZC)

    add_object("../data/crate.obj")

    E.enable_timer(true)
end

function do_click(b, s)
    if b == 1 then
        tumble = s
    end
    if b == 3 then
        scale  = s
    end

    return true
end

function do_point(dx, dy)
    if tumble then
        rot_x = rot_x + dy
        rot_y = rot_y + dx

        if rot_x >  90.0 then rot_x =  90 end
        if rot_x < -90.0 then rot_x = -90 end

        E.set_entity_rotation(pivot, rot_x, rot_y, 0)

        return true
    end

    if scale then
        zoom = zoom + dy / 500

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
            E.set_entity_position(camera, 0.0, 0.0, 0.0)
            E.set_entity_rotation(camera, 0.0, 0.0, 0.0)
            rot_x = 0
            rot_y = 0
            pan_x = 0
            pan_y = 0
            pan_z = 0
            return true
        end

        if k == E.key_F7 then
            E.set_camera_stereo(camera, E.stereo_mode_none,
                                -d, 0, 0, d, 0, 0)
            return true
        end
        if k == E.key_F8 then
            E.set_camera_stereo(camera, E.stereo_mode_tile,
                                -d, 0, 0, d, 0, 0)
            return true
        end
        if k == E.key_F9 then
            E.set_camera_stereo(camera, E.stereo_mode_red_blue,
                                -d, 0, 0, d, 0, 0)
            return true
        end

	if k == E.key_insert then
            rot_dy = rot_dy + 1
	end
	if k == E.key_delete then
            rot_dy = rot_dy - 1
	end
    end

    return false
end

-------------------------------------------------------------------------------

do_start()
