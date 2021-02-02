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
    { 'Text', 'Canvas', 'Object', 'Camera', 'Audio', 'Input', 'Other', 'Nothing' }, true)
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
    for i,v in pairs(current_map.objects) do
        print(i, ': ', v.name)
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
    game:set_float_config('graphics.brightness', -0.1)
    text(o, "Changed brightness to -0.1"):wait()
    game:set_float_config('graphics.brightness', 0.1)
    text(o, "Changed brightness to 0.1"):wait()
    game:set_float_config('graphics.brightness', 0.0)
    game:set_float_config('graphics.contrast', 0.5)
    text(o, "Changed contrast to 0.5"):wait()
    game:set_float_config('graphics.contrast', 3.0)
    text(o, "Changed contrast to 3.0"):wait()
    game:set_float_config('graphics.contrast', 1.0)
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
    camera:zoom(2, 1000):wait()
    text(o, "...and out"):wait()
    camera:zoom(old_mag, 1000):wait()
    local res = game.monitor_resolution
    print('Monitor resolution: ' .. res.x .. 'x' .. res.y)
    print('List of supported resolutions: ')
    for i, res in ipairs(game.monitor_resolutions) do
        print(i .. ' - ' .. res.x .. 'x' .. res.y)
    end
    local bounds = camera.position_bounds
    print('Camera bounds: ' .. bounds.x .. ', ' .. bounds.y .. ', ' .. bounds.w .. ', ' .. bounds.h)
    local cpos = camera:get_centered_position(o)
    print('Centered position for object: ' .. cpos.x .. ', ' .. cpos.y)
    local w = game.width
    local h = game.height
    text(o, "Changing screen size to 800, 800"):wait()
    game:set_size(800, 800)
    wait(500)
    text(o, "Changing it back to " .. w .. ", " .. h):wait()
    game:set_size(w, h)
    wait(500)
    text(o, 'Going to fullscreen mode'):wait()
    game.fullscreen = true
    local old_mode = game:get_config('graphics.scale-mode')
    text(o, 'Setting scale mode from ' .. old_mode .. ' to stretch'):wait()
    game:set_string_config('graphics.scale-mode', 'stretch')
    text(o, 'And reverting it back to ' .. old_mode .. ' scale mode'):wait()
    game:set_string_config('graphics.scale-mode', old_mode)
    text(o, 'Going back to windowed mode'):wait()
    game.fullscreen = false
    text(o, "Moving camera"):wait()
    camera:move(UP, 120, 1):wait()
    camera:move_to(player, 1):wait()
    camera:track_object(player)
    text(o, "Tinting screen back"):wait()
    camera:tint_screen(Color(0.4, 0.2, 0.6, 0), 500):wait()
elseif c.selected == 5 then
    -- Audio
    local music = game.playing_music
    text(o, "Pausing music " .. music.filename):wait()
    music:pause()
    local sound_file = 'data/as3sfxr_menu_select.wav'
    text(o, 'Playing sound effect ' .. sound_file):wait()
    Sound(sound_file):play()
    text(o, 'Playing sound effect with half volume and pitch'):wait()
    local sound = Sound(sound_file)
    sound.volume = 0.5
    sound.pitch = 0.5
    print('Volume: ' .. sound.volume .. ', pitch: ' .. sound.pitch)
    sound:play()
    text(o, 'Changing global sound volume then playing again'):wait()
    local old_sound_volume = game.global_sound_volume
    game.global_sound_volume = 0.5
    Sound(sound_file):play()
    text(o, 'Playing sound effect again with half volume and pitch'):wait()
    sound:play()
    text(o, 'Changing global sound volume via configuration'):wait()
    game:set_float_config('audio.sound-volume', 0.25)
    wait(1)
    Sound(sound_file):play()
    text(o, 'Resuming music'):wait()
    music:play()
    text(o, 'Changing global music volume'):wait()
    local old_music_volume = game.global_music_volume
    game.global_music_volume = 0.3
    text(o, "Resetting music volume"):wait()
    game.global_sound_volume = old_sound_volume
    game.global_music_volume = old_music_volume
elseif c.selected == 6 then
    -- Input
    print('Controller names: ')
    for id, name in pairs(game.gamepad_names) do
        print(id .. ' - ' .. name)
    end
    text(o, "Waiting for action key to be pressed"):wait()
    game:wait_for_input('a')
    text(o, 'Waiting for any key to be pressed'):wait()
    game:wait_for_input()
    text(o, "Printing out triggered keys. Press SHIFT to stop"):wait()
    local done = false
    while not done do
        local triggered_keys = game.triggered_keys
        if #triggered_keys > 0 then
            local keys = ''
            for i, key in ipairs(triggered_keys) do
                if key == '' then
                    keys = keys .. ' ' .. 'Unknown'
                elseif key == 'SHIFT' then
                    done = true
                    break
                else
                    keys = keys .. ' ' .. key
                end
            end
            if not done then
                text(o, 'Triggered keys:' .. keys):wait()
            end
        end
        wait(1)
    end
    text(o, "Starting to record character input. Press ENTER to stop"):wait()
    done = false
    game:begin_character_input()
    while not done do
        wait(0)
        for i, key in ipairs(game.triggered_keys) do
            if key == 'ENTER' then
                done = true
            end
        end
   end
   local char_input = game:end_character_input()
   text(o, "You entered: " .. char_input):wait()
elseif c.selected == 7 then
    -- Other
    local files = filesystem.list_directory('.')
    print('Files in current folder:')
    for _, file in ipairs(files) do
        local file_type = filesystem.is_directory(file) and 'directory' or 'file'
        print('\t- ' .. file .. ' - ' .. file_type)
    end
    text(o, "Are we in debug mode? " .. (game.debug and "Yes!" or "No!")):wait()
    local pause_unfocused = game:get_config('game.pause-unfocused')
    text(o, "Config title: " .. game:get_config('game.title')
        .. '\nscreen width: ' .. game:get_config('graphics.screen-width')
        .. '\npause-unfocused: ' .. pause_unfocused
        .. '\naxis-sensitivity: ' .. game:get_config('controls.axis-sensitivity')):wait()
    text(o, "Changing screen width and screen height config"):wait()
    local game_size = Vec2(game.width, game.height)
    game:set_int_config('graphics.screen-width', 800)
    game:set_int_config('graphics.screen-height', 800)
    print('game_width: ' .. game.width .. ', height: ' .. game.height)
    text(o, "Reverting screen width and screen height config"):wait()
    game:set_int_config('graphics.screen-width', game_size.x)
    game:set_int_config('graphics.screen-height', game_size.y)
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
    print('Original table:')
    print(print_table(tbl))
    game:save('data/test_save.txt', tbl)
    local tbl2 = game:load('data/test_save.txt')
    text(o, "And loading it back. Loaded f.x = " .. tbl2.f.x):wait()
    print('Loaded file:')
    print(print_table(tbl2))
    text(o, "Copying file"):wait()
    local copy_name = "data/test_save2.txt"
    filesystem.copy("data/test_save.txt", copy_name)
    local tbl3 = game:load(copy_name)
    print('Loaded copied file:')
    print(print_table(tbl3))
    print('File exists: ' .. tostring(filesystem.exists(copy_name)))
    text(o, 'Removing the copy'):wait()
    filesystem.remove(copy_name)
    print('File exists: ' .. tostring(filesystem.exists(copy_name)))
    text(o, "Type of Object:" .. type(o) .. " - type of Vec2: " .. type(Vec2(0,0))):wait()
    text(o, "UP | RIGHT = " .. bit.bor(UP, RIGHT)):wait()
end
text(o, "That's all!"):wait()
player.disabled = false

