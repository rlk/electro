
-------------------------------------------------------------------------------

pos_x =    0.0
pos_y =   15.5
pos_z = 9200.0
rot_x =    0.0
rot_y =    0.0
dist  = 1000.0

btn    = { }
btn[1] = false
btn[2] = false
btn[3] = false
btn[4] = false
btn[5] = false

last_x = 0.0
last_y = 0.0

-------------------------------------------------------------------------------

function do_click(b, s)

        if (b == 4) then
                dist = dist + 1.0
                camera_dist(dist)
        end
        if (b == 5) then
                dist = dist - 1.0
                camera_dist(dist)
        end
                
        btn[b] = s

        print(b, s)
end

function do_point(x, y)
        local dx = last_x - x
        local dy = last_y - y

        last_x = x
        last_y = y

        if btn[1] then
                rot_x = rot_x + 180 * dy / 500.0
                rot_y = rot_y + 180 * dx / 500.0

                if rot_x >   90 then rot_x =  90 end
                if rot_x <  -90 then rot_x = -90 end
                if rot_y >  180 then rot_y = rot_y - 360 end
                if rot_y < -180 then rot_y = rot_y + 360 end

                camera_turn(rot_x, rot_y, 0)
                scene_draw()
        end
end



-------------------------------------------------------------------------------
