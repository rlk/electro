
image_file = "venus.jpg"

view_x = 0
view_y = 0
view_w = 0
view_h = 0

camera  = nil
numbers = { }
sprites = { }

curr_i  = 4
curr_j  = 4
time    = 0

-------------------------------------------------------------------------------

function move_sprite(sprite, i, j)
    E.set_entity_position(sprite, view_x + view_w * (j - 1) / 4 + view_w / 8,
                                  view_y + view_h * (4 - i) / 4 + view_h / 8, 0)
end

function solved()
    local k = 1

    for i = 1, 4 do
        for j = 1, 4 do
            if numbers[i][j] == k then
                k = k + 1
            else
                return false
            end
        end
    end
    return true
end

function move_piece(di, dj)
    local next_i = curr_i + di
    local next_j = curr_j + dj

    -- Confirm that the move is valid.

    if 1 <= next_i and next_i <= 4 and
       1 <= next_j and next_j <= 4 then

        -- Swap the moving piece with the blank piece.

        temp                    = numbers[curr_i][curr_j]
        numbers[curr_i][curr_j] = numbers[next_i][next_j]
        numbers[next_i][next_j] = temp

        -- Set the new on-screen locations of the moving sprites.

        move_sprite(sprites[numbers[curr_i][curr_j]], curr_i, curr_j)
        move_sprite(sprites[numbers[next_i][next_j]], next_i, next_j)

        curr_i = next_i
        curr_j = next_j

        -- Set the visibility of the 16th piece to indicate solution.

        E.set_entity_flag(sprites[16], E.entity_flag_hidden, not solved())

        return true
    else
        return false
    end
end

function move_random()
    while true do
        local d = math.random(-1, 1)

        if math.random(0, 1) == 0 then
            if move_piece(d, 0) then
                return
            end
        else
            if move_piece(0, d) then
                return
            end
        end
    end
end

-------------------------------------------------------------------------------

function do_start()

    view_x, view_y, view_w, view_h = E.get_display_union()

    camera = E.create_camera(E.camera_type_orthogonal)
    E.set_entity_flag(camera, E.entity_flag_unlit, true);

    -- Initialize an array that maps rows and columns onto puzzle pieces.

    numbers[1] = {  1,  2,  3,  4 }
    numbers[2] = {  5,  6,  7,  8 }
    numbers[3] = {  9, 10, 11, 12 }
    numbers[4] = { 13, 14, 15, 16 }

    -- Add 16 sprites and scale each to 1/16th the size of the display.

    for i = 1, 16 do
        sprites[i] = E.create_sprite(image_file)

        local sw, sh = E.get_sprite_size(sprites[i])

        E.parent_entity(sprites[i], camera)
        E.set_entity_scale(sprites[i], 0.25 * view_w / sw,
                                       0.25 * view_h / sh, 1.0)
    end

    -- Assign 1/16th of the image to each sprite.

    for i = 1, 4 do
        for j = 1, 4 do
            local x0 = (j - 1) / 4
            local x1 = (j - 0) / 4
            local y0 = (4 - i) / 4
            local y1 = (5 - i) / 4

            move_sprite(sprites[numbers[i][j]], i, j)
            E.set_sprite_bounds(sprites[numbers[i][j]], x0, x1, y0, y1)
        end
    end

    return true
end

function do_keyboard(k, s)
    if s then
        -- If an arrow key has been pressed, move a puzzle piece.

        if k == 276 then move_piece( 0,  1) end
        if k == 275 then move_piece( 0, -1) end
        if k == 273 then move_piece( 1,  0) end
        if k == 274 then move_piece(-1,  0) end

        -- If autoplay has been switched, toggle the idle function.

        if k == 283 then E.enable_timer(true)  end
        if k == 284 then E.enable_timer(false) end
    end
    return true
end

function do_timer(dt)
    time = time + dt

    if time > 0.25 then
        move_random()
        time = 0
      return true
    end

    return false
end

-------------------------------------------------------------------------------

do_start()
