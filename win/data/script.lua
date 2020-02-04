function print_table(tt, indent, done)
  done = done or {}
  indent = indent or 0
  if type(tt) == "table" then
    local result = string.rep(" ", indent) .. '{\n'
    indent = indent + 2
    for key, value in pairs (tt) do
      result = result .. string.rep(" ", indent) -- indent it
      if type (value) == "table" and not done [value] then
        done [value] = true
        result = result ..  key .. " =\n";
        result = result ..  print_table(value, indent + 2, done)
        result = result ..  string.rep(" ", indent) -- indent it
        result = result ..  "\n";
      elseif "number" == type(key) then
        result = result ..  string.format("\"%s\"\n", tostring(value))
      else
        result = result ..  string.format(
            "%s = \"%s\"\n", tostring(key), tostring(value))
       end
    end
    indent = indent - 2
    return result .. string.rep(" ", indent) ..  '}'
  else
    return tostring(tt) .. "\n"
  end
end

player.disabled = true
local o = current_map:get_object("jimbo")
local c = choices(o, "What do you want to test?",
    { 'Text', 'Canvas', 'Object', 'Camera', 'Other', 'Nothing' })
c:wait()
print("Choice: ", c.selected)
if c.selected == 1 then
    -- Text
    text(o, "Showing some text"):wait()
    text(o, "{shake=0}Now...{/shake}{shake=10}This here{/shake} {shake=30}is some{/shake} {shake=50}weird{/shake}{shake=70} shaking {/shake}{shake=90}text,{/shake}{shake} isn't it?{/shake}"):wait()
    local options = Text_Options(o):set_text('Custom text options!'):set_choices({'Ok!', 'Nope'}):set_show_dashes(false):set_choice_indent(8)
    choices(options):wait()
    text(o, "{color=blue}Test?\n\n {rainbow}Hello,\nd{color=green}ea{/color}r\n world!{/rainbow}{/color}\nHow{bold} about {/bold}that?"):wait()
    text(Vec2(100, 100), "{typewriter}Slowly now...{/typewriter}", 2000):wait()
    centered_text(100, "Some centered {italic}text goes here, ha ha\nyeah {/italic}that's right!"):wait()
    wait(500)
    text(o, 'Manually showing text with a canvas'):wait()
    local text_canvas = Canvas(40, 40, "{type=bold2}This{/type} is a {bold}Canvas{/bold} {italic}test{/italic},\n will it work?")
    text_canvas.scissor_box = Rect(30, 20, 140, 60)
    text_canvas:link_font('bold2', 'data/Roboto-Bold.ttf')
    text_canvas:show()
    wait(1500)
    text(o, 'Changing text properties'):wait()
    print('Font size ' .. text_canvas.font_size)
    text_canvas.font_size = 15
    function color_to_s(color)
     return '(' .. color.r .. ', ' .. color.g .. ', ' .. color.b .. ', ' .. color.a .. ')'
    end
    print('Text color ' .. color_to_s(text_canvas.text_color))
    text_canvas.text_color = Color('red')
    print('Line height ' .. text_canvas.line_height)
    text_canvas.line_height = 20
    print('Outline width ' .. text_canvas.text_outline_width)
    text_canvas.text_outline_width = 2
    print('Outline color ' .. color_to_s(text_canvas.text_outline_color))
    text_canvas.text_outline_color = Color('yellow')
    print('Shadow offset (' .. text_canvas.text_shadow_offset.x .. ', ' .. text_canvas.text_shadow_offset.y .. ')')
    text_canvas.text_shadow_offset = Vec2(-18, -18)
    print('Shadow color ' .. color_to_s(text_canvas.text_shadow_color))
    text_canvas.text_shadow_color = Color('blue')
    wait(2000)
    print('Type: ' .. text_canvas.text_type)
    text_canvas.text_type = 'bold'
    wait(2000)
    print('Setting font')
    text_canvas:set_font('data/Roboto-Bold.ttf')
    print('Setting linked font')
    text_canvas:link_font('italic', 'data/Roboto-Italic.ttf')
    text_canvas:link_font('bold', 'data/Roboto-Regular.ttf')
    text_canvas:link_font('bold2', 'data/Roboto-Italic.ttf')
    wait(3000)
    print('Test permissive mode')
    text_canvas.permissive_tag_parsing = true
    text_canvas.text = 'Hello {italic}thing'
    wait(1000)
    text_canvas:hide()
    text_canvas = nil
elseif c.selected == 2 then
    -- Canvas
    text(o, "Showing a canvas"):wait()
    local canvas = Canvas("data/player.png", 100, 100, "#FF00FF")
    canvas.has_image_outline = true
    canvas.image_outline_color = Color('blue')
    canvas.magnification = Vec2(0, 0)
    canvas:show()
    text(o, "Updating it"):wait()
    canvas:update(200, 100, 1, 1, 180, 1, 1500):wait()
    wait(500)
    canvas:update(100, 100, 1, 1, 0, 0, 1500):wait()
    canvas = nil
    text(o, "Garbage collecting canvases"):wait()
    collectgarbage()
elseif c.selected == 3 then
    -- Object
    print("Objects:")
    local objects = current_map:get_objects()
    for i,v in ipairs(objects) do
        print(i,": ", v.name)
    end
    text(o, "Moving object"):wait()
    o:move(UP, 32, true, false):wait()
    o:move(LEFT, 32):wait()
    o:move(UP, 32):wait()
    o:move(RIGHT, 64):wait()
    o:move(DOWN, 100):wait()
    o:face(RIGHT)
    o:move(BACKWARD, 50):wait()
    text(o, "Showing a pose"):wait()
    o:show_pose("Pose Test"):wait()
    o:show_pose("Default"):wait()
    o:face(LEFT)
elseif c.selected == 4 then
    -- Camera
    text(o, "Tinting screen"):wait()
    camera:tint_screen(Color(0.4, 0.2, 0.6, 0.3), 500):wait()
    text(o, "Setting magnification"):wait()
    local old_mag = game.magnification
    game.magnification = 2
    wait(500)
    text(o, "...And resetting it"):wait()
    game.magnification = old_mag
    wait(500)
    text(o, "Gradually zooming in"):wait()
    camera:zoom(2, 3000):wait()
    text(o, "...and out"):wait()
    camera:zoom(old_mag, 3000):wait()
    print('List of supported sizes: ')
    for i, size in ipairs(game.sizes) do
        print(i .. ' - ' .. size.x .. ' x ' .. size.y)
    end
    local w = game.width
    local h = game.height
    text(o, "Changing screen size to 800, 800"):wait()
    game:set_size(800, 800)
    wait(500)
    text(o, "Changing it back to " .. w .. ", " .. h):wait()
    game:set_size(w, h)
    wait(500)
    text(o, "Moving camera"):wait()
    camera:move(UP, 120, 1):wait()
    camera:move_to(player, 1):wait()
    camera:track_object(player)
    text(o, "Tinting screen back"):wait()
    camera:tint_screen(Color(0.4, 0.2, 0.6, 0), 500):wait()
elseif c.selected == 5 then
    -- Others
    local files = list_directory_files('.')
    print('Files in current folder:')
    for _, file in ipairs(files) do
        print('\t- ' .. file)
    end
    local music = game.playing_music
    text(o, "Pausing music " .. music.filename):wait()
    music:pause()
    text(o, "Are we in debug mode? " .. (game.is_debug and "Yes!" or "No!")):wait()
    local pause_unfocused = game:get_config('game.pause-unfocused')
    text(o, "Config title: " .. game:get_config('game.title')
        .. '\nscreen width: ' .. game:get_config('game.screen-width')
        .. '\npause-unfocused: ' .. pause_unfocused
        .. '\naxis-sensitivity: ' .. game:get_config('controls.axis-sensitivity')):wait()
    text(o, "Changing screen width and screen height config"):wait()
    local game_size = Vec2(game.width, game.height)
    game:set_int_config('game.screen-width', 800)
    game:set_int_config('game.screen-height', 800)
    print('game_width: ' .. tostring(game.width) .. ', height: ' .. tostring(game.height))
    text(o, "Reverting screen width and screen height config"):wait()
    game:set_int_config('game.screen-width', game_size.x)
    game:set_int_config('game.screen-height', game_size.y)
    text(o, "Toggling game.pause-unfocused config"):wait()
    game:set_bool_config('game.pause-unfocused', pause_unfocused == "0")
    local tbl = {
        a = 'aa',
        b = 'ba',
        c = 'ca',
        d = 6.5,
        e = false,
        f = {
            x = 1,
            y = true,
            z = 'w',
        },
        g = nil,
        x = function() end
    }
    text(o, "Saving a table where f.x = 1"):wait()
    print(print_table(tbl))
    game:save('data/test_save.txt', tbl)
    tbl2 = game:load('data/test_save.txt')
    text(o, "And loading it back. Loaded f.x = " .. tbl2.f.x):wait()
    print(print_table(tbl2))
    text(o, "Type of Object:" .. type(o) .. " - type of Vec2: " .. type(Vec2(0,0))):wait()
    text(o, "UP | RIGHT = " .. bitor(UP, RIGHT)):wait()
    text(o, "Waiting 1000 ms then resuming music"):wait()
    wait(1000)
    music:play()
end
text(o, "That's all!"):wait()
player.disabled = false

