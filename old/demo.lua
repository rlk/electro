
joy_dev = 0

-------------------------------------------------------------------------------

name_text = "Varrier Demos"
help_text = "Select a demo.  Press F12 to return."

demo = {
    { "Trainer",                  "trainer",   "trainer.lua", true   },
    { "Mars", "/DEMO/varrier/mars1/bin","mpirun -np 3 varrier", false   },
    { "Video Viewer",             "movie",     "movie.lua", true     },
    { "Hipparcos Explorer",       "vortex",    "vortex.lua", true    },
--  { "NCMIR Data",               "imgplay",   "imgplay.lua", true   },
    { "Driving",                  "driving",   "driving.lua", true   },
    { "4D Julia",                 "flip",      "flip.lua", true   },
}

item = { }

selected = 1

-------------------------------------------------------------------------------

function centered_string(text, line, fill)
    local string = E.create_string(text)
    local pivot  = E.create_pivot()

    local x0, y0, z0, x1, y1, z1 = E.get_entity_bound(string)

    E.set_string_line(string, line)
    E.set_string_fill(string, fill)
    E.parent_entity(string, pivot)

    E.set_entity_position(string, -(x1 - x0) / 2, -0.5, 0)

    return pivot, (x1 - x0), (y1 - y0)
end

function set_selected(d)
    E.set_string_fill(E.get_entity_child(item[selected].O, 0), menu_brush)

    selected = (selected + d)
    selected = math.max(selected, 1)
    selected = math.min(selected, table.getn(demo))

    E.set_string_fill(E.get_entity_child(item[selected].O, 0), item_brush)
end

function run_selected()
    if demo[selected][4] then
        E.nuke()
        E.chdir(demo[selected][2])
        dofile(demo[selected][3])
    else
        E.enable_timer(false)
        E.pushdir(demo[selected][2])
        os.execute(demo[selected][3])
        E.popdir()
    end
end

-------------------------------------------------------------------------------

time     = 0
centered = true

function do_timer(dt)
    local jx, jy = E.get_joystick(joy_dev)

    time = time + dt

    local r = math.min(dsp_w / 3, dsp_h / 3)
    local x = r * math.sin(time)
    local y = (dsp_y1 + dsp_y0) / 2
    local k = 10 + 3 * math.sin(time / 4)

    E.set_entity_position(light, x, y, dsp_z + dsp_h / 2)
    E.set_entity_scale(logo, k, k, k)

    if centered and jy < -0.5 then
        set_selected(-1)
        centered = false
    end
    if centered and jy >  0.5 then
        set_selected( 1)
        centered = false
    end
    if -0.5 < jy and jy < 0.5 then
        centered = true
    end

    return true
end

function do_keyboard(k, s)
    if varrier_keyboard then
        if varrier_keyboard(k, s, camera) then
            return true
        end
    end

    if s then E.enable_timer(true) end

    if s and k == E.key_up     then set_selected(-1) end
    if s and k == E.key_down   then set_selected( 1) end
    if s and k == E.key_return then run_selected()   end
    return true
end

function do_joystick(d, b, s)

    if s then
        run_selected()
    end
    return true        
end

function do_start()
--  dsp_x, dsp_y, dsp_w, dsp_h = E.get_display_union()
    dsp_x0, dsp_y0, dsp_z0, dsp_x1, dsp_y1, dsp_z1 = E.get_display_bound()

    dsp_w = dsp_x1 - dsp_x0
    dsp_h = dsp_y1 - dsp_x0
    dsp_z = (dsp_z1 + dsp_z0) / 2

    local H = 0
    local W = 0
    local N = table.getn(demo)

    logo_brush = E.create_brush()
    line_brush = E.create_brush()
    name_brush = E.create_brush()
    menu_brush = E.create_brush()
    item_brush = E.create_brush()

    E.set_brush_color(logo_brush, 0.0, 0.0, 0.0, 1.0,
                                  1.0, 1.0, 1.0, 1.0, 0, 0, 0, 0, 100)
    E.set_brush_color(line_brush, 1.0, 1.0, 1.0, 0.3,
                                  0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0, 100)
    E.set_brush_color(name_brush, 1.0, 1.0, 0.0, 0.7,
                                  1.0, 1.0, 1.0, 1.0, 0, 0, 0, 0,  10)
    E.set_brush_color(menu_brush, 0.6, 0.6, 0.6, 0.5,
                                  0.5, 0.5, 0.5, 1.0, 0, 0, 0, 0, 100)
    E.set_brush_color(item_brush, 1.0, 0.6, 0.0, 0.7,
                                  0.5, 0.5, 0.5, 0.5, 0, 0, 0, 0,  10)

    E.set_background(0, 0.5, 0, 0, 0, 0)

--  camera = E.create_camera(E.camera_type_orthogonal)
    camera = E.create_camera(E.camera_type_perspective)
    light  = E.create_light(E.light_type_positional)
    menu   = E.create_pivot()

    E.parent_entity(light, camera)
    E.parent_entity(menu,  light)

    E.set_entity_flags(config, E.entity_flag_hidden, true)

    -- Find the maximum width and height of all menu items.

    E.set_typeface("data/VeraBd.ttf", 0.0001, 0.03125)
    table.foreachi(demo,
                   function (i, line)
                       local O, w, h = centered_string(line[1], line_brush,
                                                                menu_brush)
                       item[i]   = { }
                       item[i].O =  O
                       item[i].w =  w
                       item[i].h =  h

                       H = math.max(H, h)
                       W = math.max(W, w)
                   end)

    -- Position the menu on screen.

    k = math.min(0.60 * dsp_w / W, 0.50 * dsp_h / (H * N))

    E.set_entity_scale(menu, k, k, k)
--  E.set_entity_position(menu, dsp_w / 2, 3 * dsp_h / 4, dsp_z)
    E.set_entity_position(menu, 0, (dsp_y1 + dsp_y0) / 2 + 0.33, dsp_z)

    -- Position each item within the menu.

    table.foreachi(demo,
                   function (i, line)
                       E.set_entity_position(item[i].O, 0, -i * H, 0)
                       E.parent_entity      (item[i].O, menu)
                   end)

    -- Position the titles.

    local name, w, h = centered_string(name_text, line_brush, name_brush)
    E.set_entity_scale   (name, 2, 2, 2)
    E.set_entity_position(name, 0, h, 0)
    E.parent_entity      (name, menu)

    local help, w, h = centered_string(help_text, line_brush, name_brush)
    E.set_entity_scale   (help, 0.5, 0.5, 0.5)
    E.set_entity_position(help, 0, -H * (N + 1), 0)
    E.parent_entity      (help, menu)

    -- Create the background logo.

    E.set_typeface("data/Electro.ttf", 0.0001, 1 / 256)
    logo, w, h = centered_string("E", line_brush, logo_brush)
    E.parent_entity(logo, menu)

    E.set_entity_position(logo, 0, -N * H / 2, -1)
    E.set_entity_scale(logo, 2 * N, 2 * N, 2 * N)
    E.set_entity_alpha(logo, 0.1)

    set_selected(0)
    E.enable_timer(true)
end

-------------------------------------------------------------------------------

do_start()
