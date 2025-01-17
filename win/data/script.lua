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
    { 'Text', 'Canvas', 'Object', 'Map', 'Camera', 'Audio', 'Input', 'Filesystem', 'Other', 'Nothing' }, true)
c:wait()
print("Choice: ", c.selected)
if c.selected == 1 then
    -- Text
    text(o, "Showing some text"):wait()
    text(o, "{shake=0}Now...{/shake}{shake=10}This here{/shake} {shake=30}is some{/shake} {shake=50}weird{/shake}{shake=70} shaking {/shake}{shake=90}text,{/shake}{shake} isn't it?{/shake}"):wait()
    local options = Text_Options(o)
        :set_text('Text options (method chaining)')
        :set_choices({'Ok!', 'Nope'})
        :set_show_dashes(false)
        :set_choice_indent(8)
    local cc = choices(options)
    cc:wait()
    print('Selected: ' .. cc.selected)
    options = {
        text = 'Cancelable options (table)',
        choices = { 'Yes', 'No' },
        object = o,
        position_type = bit.bor(Text_Position_Type.centered_x, Text_Position_Type.exact_y),
        show_dashes = false,
        cancelable = true,
        choice_indent = 6,
        canvas_priority = 666,
        fade_in_duration = 500,
        fade_out_duration = 250,
        background_color = Color('red'),
        typewriter_on = true,
        typewriter_delay = 50,
        typewriter_skippable = true,
        typewriter_sound = 'data/as3sfxr_menu_click.wav',
    }
    cc = choices(options)
    cc:wait()
    print('Selected: ' .. cc.selected)
    text(o, "{color=blue}Test?\n\n {rainbow}Hello,\nd{color=green}ea{/color}r\n world!{/rainbow}{/color}\nHow{bold} about {/bold}that?"):wait()
    text(Vec2(100, 100), "{typewriter}Slowly showing some text...{/typewriter}", 3000):wait()
    centered_text(100, "Some centered {italic}text goes here, ha ha\nyeah {/italic}that's right!"):wait()
    local pos_type = bit.bor(bit.bor(Text_Position_Type.centered_x, Text_Position_Type.exact_y),
        bit.bor(Text_Position_Type.camera_relative, Text_Position_Type.always_visible))
    options = {
        text = "Some centered text around x, ha ha\nyeah that's right!",
        centered = true,
        position_type = pos_type,
        position = Vec2(400, 400),
    }
    text(options):wait()
    local canvas_choice = choices(o, 'Test canvas text?', { 'yes', 'no' }, true)
    canvas_choice:wait()
    if canvas_choice.selected == 1 then
        text(o, 'Manually showing text with a canvas'):wait()
        local text_canvas = Text_Canvas(40, 40, "{type=bold2}This{/type} is a {bold}Canvas{/bold} {italic}test{/italic},\n will it work?")
        text_canvas.scissor_box = Rect(30, 20, 140, 60)
        text_canvas:link_font('bold2', 'data/Roboto-Bold.ttf')
        text_canvas:show()
        wait(1500)
        text(o, 'Changing text properties'):wait()
        print('Setting centered')
        text_canvas.centered = true
        game:wait_for_input()
        text_canvas.centered = false
        print('Font size ' .. text_canvas.font_size)
        text_canvas.font_size = 15
        function color_to_s(color)
         return '(' .. color.r .. ', ' .. color.g .. ', ' .. color.b .. ', ' .. color.a .. ')'
        end
        print('Text color ' .. color_to_s(text_canvas.color))
        text_canvas.color = Color('red')
        print('Line height ' .. text_canvas.line_height)
        text_canvas.line_height = 20
        print('Outline width ' .. text_canvas.outline_width)
        text_canvas.outline_width = 2
        print('Outline color ' .. color_to_s(text_canvas.outline_color))
        text_canvas.outline_color = Color('yellow')
        print('Shadow offset (' .. text_canvas.shadow_offset.x .. ', ' .. text_canvas.shadow_offset.y .. ')')
        text_canvas.shadow_offset = Vec2(-18, -18)
        print('Shadow color ' .. color_to_s(text_canvas.shadow_color))
        text_canvas.shadow_color = Color('blue')
        game:wait_for_input()
        print('Type: ' .. text_canvas.font_type)
        text_canvas.font_type = 'bold'
        game:wait_for_input()
        print('Setting font')
        text_canvas:set_font('data/Roboto-Bold.ttf')
        print('Setting linked font')
        text_canvas:link_font('italic', 'data/Roboto-Italic.ttf')
        text_canvas:link_font('bold', 'data/Roboto-Regular.ttf')
        text_canvas:link_font('bold2', 'data/Roboto-Italic.ttf')
        game:wait_for_input()
        print('Test permissive mode')
        text_canvas.permissive_tag_parsing = true
        text_canvas.text = 'Hello {italic}thing'
        game:wait_for_input()
        text_canvas:hide()
        text_canvas = nil
    end
elseif c.selected == 2 then
    -- Canvas
    text(o, "Showing and updating canvas"):wait()
    local canvas = Image_Canvas("data/player.png", 100, 100, "#FF00FF")
    canvas.origin = Vec2(0.5, 0.5)
    canvas.outline_color = Color('blue')
    canvas.magnification = Vec2(0, 0)
    canvas:show()
    local cv_c = canvas:update {
        position = Vec2(200, 100),
        magnification = Vec2(2, 1),
        angle = 180,
        duration = 1500
    }
    cv_c:wait()
    wait(500)
    cv_c = canvas:update {
        position = Vec2(100, 100),
        magnification = Vec2(1, 1),
        angle = 0,
        opacity = 0,
        duration = 1500
    }
    canvas:update_color(Color('yellow'), 1500)
    cv_c:wait()

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
    local move_options = {
        direction = UP,
        distance = 32,
        change_facing = false,
    }
    o:move(move_options):wait()
    o:move(LEFT, 32):wait()
    o:move(UP, 32):wait()
    o:move(RIGHT, 64):wait()
    o:move(DOWN, 100):wait()
    o:face(RIGHT)
    o:move(BACKWARD, 30):wait()
    o:move(FORWARD, 20):wait()
    text(o, "Showing a pose"):wait()
    o:show_pose("Pose Test"):wait()
    text(o, "Showing an infinite pose until complete"):wait()
    local p_c = o:show_pose("Infinite Pose")
    wait(3000)
    p_c:wait()
    o:show_pose("Default"):wait()
    o:face(LEFT)
    text(o, "Updating color and opacity"):wait()
    local old_obj_opacity = o.opacity
    local old_obj_color = o.color
    local op_c = o:update_opacity(0.5, 250)
    o:update_color(Color('green'), 250)
    op_c:wait()
    text(o, "Resetting color and opacity"):wait()
    o.opacity = old_obj_opacity
    o.color = old_obj_color
elseif c.selected == 4 then
    -- Map
    local snow_layer = current_map:get_image_layer('snow')
    local old_velocity = snow_layer.velocity
    text(o, "Slowing down snow layer"):wait()
    snow_layer:update_velocity(old_velocity / 10, 500):wait()
    local old_opacity, old_color = snow_layer.opacity, snow_layer.color
    text(o, "Updating opacity of snow layer"):wait()
    snow_layer:update_opacity(1, 500):wait()
    text(o, "Updating color of snow layer"):wait()
    snow_layer:update_color(Color('red'), 500):wait()
    text(o, "Resetting color, opacity and velocity of snow layer"):wait()
    snow_layer.opacity = old_opacity
    snow_layer.color = old_color
    snow_layer.velocity = old_velocity
    text(o, "Updating object layer color and opacity"):wait()
    local object_layer = current_map:get_object_layer('objects')
    old_opacity = object_layer.opacity
    old_color = object_layer.tint_color
    local op_c = object_layer:update_opacity(0.25, 250)
    object_layer:update_color(Color('blue'), 250)
    op_c:wait()
    text(o, "Resetting object layer color and opacity"):wait()
    object_layer.opacity = old_opacity
    object_layer.tint_color = old_color
elseif c.selected == 5 then
    -- Camera
    text(o, "Changing centered camera offset"):wait()
    camera.object_center_offset = Vec2(50, -50)
    wait(1000)
    camera.object_center_offset = Vec2(0, 0)
    text(o, "Starting horitontal screen shake"):wait()
    camera:start_shaking(3, 14)
    wait(250)
    camera:cease_shaking()
    camera:shake_screen(3, 14, 250):wait()
    text(o, "Stopping multi-directional screen shake"):wait()
    camera:start_shaking(Vec2(2, 3), Vec2(12, 13))
    wait(250)
    camera:cease_shaking()
    camera:shake_screen(Vec2(3, 2), Vec2(-14, 13), 250):wait()
    text(o, "Done shaking. Changing graphic config"):wait()
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
    game:set_float_config('graphics.saturation', 0.5)
    text(o, "Changed saturation to 0.5"):wait()
    game:set_float_config('graphics.saturation', 2.0)
    text(o, "Changed saturation to 2.0"):wait()
    game:set_float_config('graphics.saturation', 1.0)
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
elseif c.selected == 6 then
    -- Audio
    local audio_player = game.audio_player
    local music = audio_player.playing_music
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
    local old_sound_volume = audio_player.global_sound_volume
    audio_player.global_sound_volume = 0.3
    game.audio_player:play_sound(sound_file)
    text(o, 'Playing sound effect again with half volume and pitch'):wait()
    game.audio_player:play_sound(sound_file, 0.5, 0.5)
    text(o, 'Changing global sound volume via configuration'):wait()
    game:set_float_config('audio.sound-volume', 0.25)
    wait(1)
    Sound(sound_file):play()
    text(o, 'Resuming and fading in music'):wait()
    music.volume = 0
    music:play()
    music:fade(1, 2000):wait()
    text(o, 'Changing global music volume'):wait()
    local old_music_volume = audio_player.global_music_volume
    audio_player.global_music_volume = 0.3
    text(o, "Resetting music volume"):wait()
    audio_player.global_sound_volume = old_sound_volume
    audio_player.global_music_volume = old_music_volume
elseif c.selected == 7 then
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
                    keys = keys .. ' ' .. key .. ' (' .. game:get_key_name(key) .. ')'
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
elseif c.selected == 8 then
    -- Filesystem
    local fc = choices(o, "Which filesystem?", { "Game", "User" })
    fc:wait()
    local filesystem = fc.selected == 1 and game.game_data_filesystem
        or game.user_data_filesystem
    local function print_size(size)
        local descriptor
        if size < 1024 then
            descriptor = ' B'
        elseif size < (1024 * 1024) then
            descriptor = 'KB'
            size = math.floor(size / 1024 + 0.5)
        else
            descriptor = 'MB'
            size = math.floor(size / (1024 * 1024) + 0.5)
       end
       return string.format('%3d %s', size, descriptor)
    end
    local function test_list(method, processor)
        print('Files in current folder (' .. method .. '):')
        local files = filesystem[method](filesystem, '.')
        local processed_files = {}
        for _, file in ipairs(files) do
            table.insert(processed_files, processor(file))
        end
        table.sort(processed_files, function(a, b) return a.timestamp < b.timestamp end)
        for _, f in ipairs(processed_files) do
            print('\t- ' .. f.timestamp .. '\t' .. f.date_time .. ' ' .. f.is_dst
                .. '\t' .. print_size(f.size) .. '\t' .. f.name .. ' (' .. f.type .. ')')
        end
    end
    local function process_tm(tm)
        local date_str = string.format('%d-%02d-%02d %02d:%02d:%02d', tm.year, tm.month, tm.month_day,
            tm.hour, tm.minute, tm.second)
        local dst = tm.is_dst and '(DST!)' or (tm.is_dst == false and '(NDST)' or '(UNK?)')
        return date_str, dst
    end
    local function basic_processor(file_name)
        local file_type = filesystem:is_directory(file_name) and 'directory' or 'file'
        local ms, tm = filesystem:last_write_time(file_name)
        local date_str, dst = process_tm(tm)
        return {
            name = file_name,
            type = file_type,
            timestamp = ms,
            date_time = date_str,
            is_dst = dst,
            size = filesystem:file_size(file_name),
        }
    end
    function detailed_processor(file_info)
        local file_type = file_info.is_directory and 'directory' or 'file'
        local ms = file_info.timestamp
        local tm = file_info.calendar_time
        local date_str, dst = process_tm(tm)
        return {
            name = file_info.name,
            type = file_type,
            timestamp = ms,
            date_time = date_str,
            is_dst = dst,
            size = filesystem:file_size(file_info.name),
        }  
    end
    test_list('list_directory', basic_processor)
    test_list('list_detailed_directory', detailed_processor)
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
    local header = {
        n = 5,
        m = { w = 'z' }
    }

    if fc.selected == 2 then
        -- Writable filesystem
        local folder = game.data_folder
        text(o, "User data folder: " .. folder.version_path):wait()
        print('Base path: ' .. folder.base_path)
        print('Game path: ' .. folder.game_path)

        text(o, "Saving a table where f.x = 1"):wait()
        print('Original table:')
        print(print_table(tbl))
        print('Original header:')
        print(print_table(header))

        folder:save('data/test_save.txt', tbl)
        folder:save('data/test_save_header.txt', tbl, header)

        print('Loaded file without header:')
        local tbl2, header2 = folder:load('data/test_save.txt')
        text(o, "And loading it back. Loaded f.x = " .. tbl2.f.x):wait()
        print(print_table(tbl2))
        print(print_table(header2))
        print('Loaded file with header:')
        tbl2, header2 = folder:load('data/test_save_header.txt')
        print(print_table(tbl2))
        print(print_table(header2))
        print('Loaded header only:')
        print(print_table(folder:load_header('data/test_save_header.txt')))

        text(o, "Copying file"):wait()
        local copy_name = "data/test_save2.txt"
        filesystem:copy("data/test_save.txt", copy_name)
        local tbl3 = folder:load(copy_name)
        print('Loaded copied file:')
        print(print_table(tbl3))
        print('File exists: ' .. tostring(filesystem:exists(copy_name)))
        text(o, 'Removing the copy'):wait()
        filesystem:remove(copy_name)
        print('File exists: ' .. tostring(filesystem:exists(copy_name)))

        text(o, "Renaming file"):wait()
        local old_name = "data/test_save.txt"
        local new_name = "data/test_save2.txt"
        filesystem:rename(old_name, new_name)
        local tbl3 = folder:load(new_name)
        print('Loaded renamed file:')
        print(print_table(tbl3))
        print('Old name exists: ' .. tostring(filesystem:exists(old_name)))
        print('New name exists: ' .. tostring(filesystem:exists(new_name)))
        text(o, 'Renaming it back'):wait()
        filesystem:rename(new_name, old_name)
        print('Old name exists: ' .. tostring(filesystem:exists(old_name)))
        print('New name exists: ' .. tostring(filesystem:exists(new_name)))
    end
elseif c.selected == 9 then
    -- Other
    text(o, "Are we in debug mode? " .. (game.debug and "Yes!" or "No!")):wait()
    local pause_unfocused = game:get_config('game.pause-unfocused')
    text(o, "Config title: " .. game:get_config('game.title')
        .. '\nscreen width: ' .. game:get_config('graphics.screen-width')
        .. '\npause-unfocused: ' .. pause_unfocused
        .. '\naxis-sensitivity: ' .. game:get_config('controls.axis-sensitivity')):wait()
    if not game.fullscreen then
        text(o, "Changing windowed width and windowed height config"):wait()
        local game_size = Vec2(game.width, game.height)
        game:set_int_config('graphics.window-width', 800)
        game:set_int_config('graphics.window-height', 800)
        wait(10)
        print('game_width: ' .. game.width .. ', height: ' .. game.height)
        text(o, "Reverting windowed width and windowed height config"):wait()
        game:set_int_config('graphics.window-width', game_size.x)
        game:set_int_config('graphics.window-height', game_size.y)
    end
    text(o, "Toggling game.pause-unfocused config"):wait()
    game:set_bool_config('game.pause-unfocused', pause_unfocused == "0")
    text(o, "Type of Object:" .. type(o) .. " - type of Vec2: " .. type(Vec2(0,0))):wait()
    text(o, "UP | RIGHT = " .. bit.bor(UP, RIGHT)):wait()
    if game.environment:can_open_store_page() then
        text(o, "Opening store page"):wait()
        game.environment:open_store_page()
    elseif game.environment:can_open_url() then
        text(o, "Opening a url"):wait()
        game.environment:open_url("https://octopuscityblues.com")
    else
        text(o, "Trying to open url in unsupported environment"):wait()
        game:open_url("https://octopuscityblues.com")
    end
end
text(o, "That's all!"):wait()
player.disabled = false
