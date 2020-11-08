local time_multiplier = tonumber(game:get_config('debug.time-multiplier'))

-- Adjusted total seconds, includes added/paused time and modified by multiplier
function total_seconds()
    return game.seconds * time_multiplier
end

-- Get the seconds portion of given time
function time_to_seconds(seconds)
    return seconds % 60
end

-- Get the minutes portion of given time
function time_to_minutes(seconds)
    return math.floor(seconds / 60) % 60
end

-- Get the hours portion of given time
function time_to_hours(seconds)
    return math.floor(seconds / 3600) % 12
end

-- Get the days portion of given time
function time_to_days(seconds)
    return math.floor(seconds / 43200) + 1
end

-- Get time without days
function time_without_days(seconds)
    return seconds - (time_to_days(seconds) - 1) * 43200
end

local class = require 'data/middleclass'
local NPC = class('NPC')

function NPC:initialize(name, data)
    -- Unique NPC name
    self.name = name
    -- Display name for dialogue etc.
    self.display_name = data.display_name or name
    -- NPC definition
    self.data = data
    -- Is NPC visible?
    self.visible = true
    -- Is NPC passthrough
    self.passthrough = false
    -- NPC direction
    self.direction = DOWN
    -- How fast game time is vs real time
    self.time_multiplier = time_multiplier
    -- Time taken for a single frame
    self.frame_time = 1000 / tonumber(game:get_config('debug.logic-fps'))
    -- Last position
    self.position = Vec2(-1, -1)
    -- Is NPC's schedule active?
    self.active = true
    -- Used to simplify NPC scheduling
    self.dummy_command = {
        is_complete = function() return true end,
        execute = function() end
    }
    -- Set schedule to first one
    for i, v in pairs(self.data.schedules) do
        self:set_schedule(i)
        break
    end
end

-- Reset keypoint state
function NPC:reset_keypoint(keypoint)
    -- Actual timestamp when keypoint was started
    keypoint.start_time = -1
    -- Current action status
    keypoint.status = 'pending'
    -- Day on which point was last completed
    keypoint.completion_day = 0
    -- Current command index
    keypoint.command_index = 1
    -- List of commands
    if not keypoint.commands then
        keypoint.commands = {}
    end
end

-- Get a schedule by name
function NPC:get_schedule(name)
    return self.data.schedules[name]
end

-- Set current schedule and reset keypoints
function NPC:set_schedule(schedule_name)
    local schedule = self.data.schedules[schedule_name]
    if not schedule then
        log_warning('Invalid schedule ' .. schedule_name .. ' for NPC ' .. self.name)
        return
    end
    if not schedule.keypoints then
        schedule.keypoints = {}
    end
    for i, keypoint in ipairs(schedule.keypoints) do
        if schedule.activation_script and not keypoint.activation_script then
            keypoint.activation_script = schedule.activation_script
        end
        self:reset_keypoint(keypoint)
    end
    -- Last executed keypoint
    self.last_keypoint = nil
    -- Estimated completion time of keypoint on other map
    self.expected_completion = -1
    -- Is the object moving to the keypoint position
    self.moving_to_keypoint = false
    -- Currently executing script command
    self.script_command = nil
    -- Currently active schedule
    self.current_schedule = schedule_name
end

-- Set current keypoint and update object if on the same map
function NPC:set_keypoint(keypoint, same_map)
    self.last_keypoint = keypoint
    if self.object and same_map then
        -- Set object's properties to keypoint's
        if keypoint.direction then
            -- Only update direction if there isn't a face command
            local has_face_command = false
            for i, command in ipairs(keypoint.commands) do
                if command.type == 'face' then
                    has_face_command = true
                    break
                end
            end
            if not has_face_command then
                self.direction = keypoint.direction
                self.object:face(self.direction)
            end
        end
        if keypoint.activation_script then
            self.object.script = keypoint.activation_script
        end
        if keypoint.pose and self.object.pose_name ~= keypoint.pose then
            self.object:show_pose(keypoint.pose)
        end
    end
end

-- Delete the NPC's map object
function NPC:delete_object(delete_from_map)
    if delete_from_map and self.object then
        current_map:delete_object(self.object)
    end
    self.object = nil
    self.script_command = nil
end

-- Mark current keypoint as completed
function NPC:complete_keypoint()
    self.last_keypoint.status = 'completed'
    self.last_keypoint.completion_day = time_to_days(total_seconds())
    self.last_keypoint.command_index = 1
    self.expected_completion = -1
end

-- Check if command fails condition or randomness criteria
function NPC:should_skip_command(command)
    local condition = not command.condition or command.condition()
    local chance = command.chance or 100
    return not condition or math.random(0, 99) >= chance
end

-- Process the next keypoint command
function NPC:process_map_command(current_time)
    if not self.object or not self.last_keypoint then return end
    self.script_command = nil
    local keypoint = self.last_keypoint
    -- Check if all commands are processed (keypoint completed)
    if keypoint.command_index > #keypoint.commands then
        self:complete_keypoint()
        return
    end

    -- Process command by type
    local command = keypoint.commands[keypoint.command_index]
    if self:should_skip_command(command) then
        -- Skip command
    elseif command.type == 'move' then
        self.script_command = Move_To_Command(self.object, command.x, command.y, true, true)
    elseif command.type == 'text' then
        local object = self.object
        if command.object then
            object = current_map:get_object(command.object)
        end
        if object then
            self.script_command = Text_Command(object, command.text,
                command.duration * 1000, current_time * 1000)
        end
    elseif command.type == 'pose' then
        local state = command.state or ''
        local direction = command.direction or self.object.direction
        self.script_command = Pose_Command(self.object, command.pose, state, direction)
    elseif command.type == 'face' then
        self.direction = command.direction
        self.object:face(command.direction)
    elseif command.type == 'teleport' then
        -- Map where the object is currently at
        self.map_name = command.map
        -- Map to which position is tied, can be different from current KP map
        self.position_map = self.map_name
        self.position = Vec2(command.x, command.y)
        self:delete_object(true)
        self:complete_keypoint()
    elseif command.type == 'wait' then
        local duration = command.duration
        if command.min or command.max then
            local min_d = command.min or 1
            local max_d = command.max or min_d
            duration = math.random(min_d, max_d)
        end
        self.script_command = Wait_Command(
            duration * 1000,
            current_time * 1000)
    elseif command.type == 'visibility' then
        self.visible = command.visible
        self.object.visible = command.visible
    elseif command.type == 'passthrough' then
        self.passthrough = command.passthrough
        self.object.passthrough = command.passthrough
    end
    
    if not self.script_command then
        self.script_command = self.dummy_command
    end
end

-- Process a move command on another map
function NPC:process_offmap_move(dest, current_time)
    self.expected_position = dest
    local distance = math.max(
        math.abs(dest.x - self.position.x),
        math.abs(dest.y - self.position.y)) * 1.25
    local delay = self.time_multiplier * distance * self.frame_time / 1000
    self.expected_completion = current_time + math.floor(delay)
end

-- Process keypoints on other maps
function NPC:process_offmap_command(current_time)
    if not self.last_keypoint then return end
    local keypoint = self.last_keypoint
    -- Check if all commands are processed (keypoint completed)
    if keypoint.command_index > #keypoint.commands then
        self:complete_keypoint()
        self.position_map = self.map_name
        self.position = self.expected_position
        return
    end

    self.expected_position = self.position
    self.expected_completion = current_time
    -- Calculate expected time and position for commands
    local command = keypoint.commands[keypoint.command_index]
    if self:should_skip_command(command) then
        -- Skip command
    elseif command.type == 'move' then
        self:process_offmap_move(Vec2(command.x, command.y), current_time)
    elseif command.type == 'face' then
        self.direction = command.direction
    elseif command.type == 'teleport' then
        self.map_name = command.map
        self.position_map = self.map_name
        self.expected_position = Vec2(command.x, command.y)
        self.position = Vec2(command.x, command.y)
        self:complete_keypoint()
    elseif command.type == 'wait' or command.type == 'text' or command.type == 'pose' then
        local duration = command.duration
        if not duration then
            if command.min then
                duration = command.min
            elseif command.max then
                duration = command.max
            else
                duration = 1
            end
        end
        self.expected_completion = current_time + duration
    elseif command.type == 'visibility' then
        self.visible = command.visible
    elseif command.type == 'passthrough' then
        self.passthrough = command.passthrough
    end
end

-- Execute a pending command, return true while processing
function NPC:execute_pending_command(current_time)
    if not self.script_command then return false end

    self.script_command:execute(current_time * 1000)
    if self.script_command:is_complete(current_time * 1000) then
        local keypoint = self.last_keypoint
        if keypoint.status ~= 'pending' then
            -- Normal keypoint command processing
            keypoint.command_index = keypoint.command_index + 1
            self:process_map_command(current_time)
        else
            -- NPC was moving to keypoint's starting position
            self.script_command = nil
        end
    end

    if self.object then
        self.position = self.object.real_position
    end

    return true
end

function NPC:parse_time(timestamp)
    local result = -1
    local colon = timestamp:find(':')
    if not colon then
        result = tonumber(timestamp)
    elseif colon > 1 then
        local hour = tonumber(timestamp:sub(1, colon - 1))
        local minute, second = 0, 0
        local rest = timestamp:sub(colon + 1)
        local colon = rest:find(':')
        if not colon then
            minute = tonumber(rest)
            second = 0
        elseif colon > 1 then
            minute = tonumber(rest:sub(1, colon - 1))
            second = tonumber(rest:sub(colon + 1))
        end
        if hour < 12 and minute < 60 and second < 60 then
            result = hour * 3600 + minute * 60 + second
        end
    end
    return result
end

-- Set timestamp and day values for keypoints
function NPC:set_keypoint_initial_time(schedule, keypoint)
    if keypoint.timestamp then
        if type(keypoint.timestamp) == 'string' then
            keypoint.timestamp = self:parse_time(keypoint.timestamp)
        end
    else
        keypoint.timestamp = -1
    end

    if not keypoint.day and schedule.day then
        keypoint.day = schedule.day
    end

    if keypoint.day then
        if keypoint.day == 'even' or keypoint.day == 'odd' then
            keypoint.day_type = keypoint.day
            keypoint.day = 1
        end
    else
        keypoint.day = 1
    end
end

-- Check if point is a good fit for starting time
function NPC:is_candidate_keypoint(keypoint, day, current_time)
    local day_match = day % keypoint.day == 0
    -- Check for even and odd days
    if keypoint.day_type == 'even' and day % 2 ~= 0 then
        day_match = false
    end
    if keypoint.day_type == 'odd' and day % 2 == 0 then
        day_match = false
    end
    local condition = not keypoint.condition or keypoint.condition(day, current_time)
    return day_match and keypoint.timestamp ~= -1 and condition and keypoint.timestamp <= current_time
end

-- Find the best keypoint matching current time (returns index and keypoint)
function NPC:find_best_keypoint(day, current_time)
    local schedule = self.data.schedules[self.current_schedule]
    local best_index = 0
    local best_keypoint = nil
    for i, keypoint in ipairs(schedule.keypoints) do
        self:set_keypoint_initial_time(schedule, keypoint)
        local better_kp = not best_keypoint or keypoint.timestamp > best_keypoint.timestamp
        if better_kp and self:is_candidate_keypoint(keypoint, day, current_time) then
            best_keypoint = keypoint
            best_index = i
        end
    end
    return best_index, best_keypoint
end

-- Get the most recent (incomplete) sequential keypoint
function NPC:advance_keypoint(keypoint, index, day)
    local schedule = self.data.schedules[self.current_schedule]
    local prior_day = keypoint.day
    local prior_day_type = keypoint.day_type
    while keypoint.status == 'completed' do
        if keypoint.completion_day ~= day then
            keypoint.status = 'pending'
        elseif keypoint.sequential and index + 1 <= #schedule.keypoints then
            index = index + 1
            keypoint = schedule.keypoints[index]
            keypoint.day = prior_day
            keypoint.day_type = prior_day_type
        else
            break
        end
    end
    return keypoint
end

-- If an NPC isn't at keypoint's position, move NPC there
-- Returns true while still moving
function NPC:move_to_keypoint(current_time)
    local old_player_passthrough = player.passthrough
    if self.last_keypoint.status == 'pending' then
        -- Set start time and timestamp if not already set
        if self.last_keypoint.start_time < 0 then
            self.last_keypoint.start_time = current_time
        end
        if self.last_keypoint.timestamp < 0 then
            self.last_keypoint.timestamp = current_time
        end
        local object_position = self.object.real_position
        local dx = math.abs(object_position.x - self.last_keypoint.x)
        local dy = math.abs(object_position.y - self.last_keypoint.y)
        if dx >= 8.0 or dy >= 8.0 then
            self.script_command = Move_To_Command(
                self.object,
                self.last_keypoint.x,
                self.last_keypoint.y,
                true,
                true)
            local simulated_ticks = 0
            local time_passed = current_time - self.last_keypoint.timestamp
            -- Simulate movement to keypoint position since timestamp
            if time_passed > 1 then
                -- Make player passable to prevent getting stuck at player position
                player.passthrough = true
                while simulated_ticks <= time_passed * 1000 do
                    local simulated_time = math.floor(current_time * 1000 + simulated_ticks)
                    self.script_command:execute(simulated_time)
                    if self.script_command:is_complete(simulated_time) then
                        self.script_command = nil
                        break
                    end
                    self.position_map = self.map_name
                    self.position = self.object.real_position
                    simulated_ticks = simulated_ticks + self.frame_time
                end
                player.passthrough = old_player_passthrough
            end
            return true
        end
        self.position_map = self.map_name
        self.position = Vec2(self.last_keypoint.x, self.last_keypoint.y)
        self.last_keypoint.status = 'started'
        self.last_keypoint.start_time = current_time
        if self.last_keypoint.activation_script then
            self.object.script = self.last_keypoint.activation_script
        end
        if self.last_keypoint.direction then
            self.object.direction = self.last_keypoint.direction
        end
    end
    return false
end

-- If NPC isn't at key point's position move it there(on another map)
-- Returns true while still moving
function NPC:move_to_offmap_keypoint(current_time)
    if self.last_keypoint.timestamp < 0 then
        self.last_keypoint.timestamp = current_time
    end
    local dx = math.abs(self.position.x - self.last_keypoint.x)
    local dy = math.abs(self.position.y - self.last_keypoint.y)
    if dx >= 8.0 or dy >= 8.0 then
        self:process_offmap_move(Vec2(self.last_keypoint.x, self.last_keypoint.y), current_time)
        self.moving_to_keypoint = true
        return true
    end
    self.expected_completion = -1
    self.expected_position = self.position
    self.last_keypoint.status = 'started'
    self.last_keypoint.start_time = current_time
    return false
end

-- Simulate on-map commands that happened since keypoint started
function NPC:simulate_commands(current_time, time_passed)
    -- Make player passable to ensure object isn't stuck at player pos
    local old_player_passthrough = player.passthrough
    player.passthrough = true
    self.last_keypoint.status = 'started'
    local simulated_ticks = 0
    self:process_map_command(current_time)
    while self.script_command and simulated_ticks <= time_passed * 1000 do
        local simulated_time = math.floor(current_time * 1000 + simulated_ticks)
        self.script_command:execute(simulated_time)
        if self.script_command:is_complete(simulated_time) then
            self.last_keypoint.command_index = self.last_keypoint.command_index + 1
            self:process_map_command(math.floor(simulated_time / 1000))
        end
        if self.object then
            self.position_map = self.map_name
            self.position = self.object.real_position
        end
        simulated_ticks = simulated_ticks + self.frame_time
    end
    player.passthrough = old_player_passthrough
end

-- The main scheduling method
function NPC:update()
    local same_map = self.map_name == current_map.filename
    if not same_map then
        self:delete_object(false)
    end
    if not self.active then
        if same_map and self.object then
            self.object.state = 'FACE'
        end
        return
    end
    local day = time_to_days(total_seconds())
    local current_time = time_without_days(total_seconds())
    if same_map and self:execute_pending_command(current_time) then
        return
    end
    local best_index, best_keypoint = self:find_best_keypoint(day, current_time)
    if not best_keypoint then
        self:delete_object(true)
        return
    end
    best_keypoint = self:advance_keypoint(best_keypoint, best_index, day)
    -- Set NPC position to keypoint map and position
    self.map_name = best_keypoint.map
    same_map = self.map_name == current_map.filename
    if self.position.x > -1 and self.position_map ~= self.map_name then
        self.position = Vec2(-1, -1)
    end
    if self.position.x < 0 then
        self.position_map = self.map_name
        self.position = Vec2(best_keypoint.x, best_keypoint.y)
    end
    -- If previous keypoint was on this map
    if same_map then
        self.expected_completion = -1
        -- If NPC isn't on the map create object at keypoint position
        if not self.object then
            local map_object = current_map:get_object(self.name)
            if map_object then
                self.object = map_object
            else
                self.object = current_map:add_new_object(
                    self.name,
                    self.data.sprite,
                    self.position,
                    self.direction)
                self.object.x = self.object.x - self.object.bounding_box.x
            end
            self.object.type = 'npc'
            self.object.direction = self.direction
        end
        self.object.visible = self.visible
        self.object.passthrough = self.passthrough
        self:set_keypoint(best_keypoint, true)
        if self:move_to_keypoint(current_time) then
            return
        end
        if self.last_keypoint.status == 'completed' then
            return
        end
        local time_passed = current_time - self.last_keypoint.start_time
        if time_passed > 1 then
            self:simulate_commands(current_time, time_passed)
        else
            self:process_map_command(current_time)
        end
    else -- If action is on another map
        -- Wait for any pending commands
        if current_time < self.expected_completion then
            return
        elseif self.expected_completion > 0 then
            if self.last_keypoint.status == 'started' then
                -- Previous command completed, process next command
                self.position = self.expected_position
                self.last_keypoint.command_index = self.last_keypoint.command_index + 1
            elseif self.moving_to_keypoint then
                -- Reached keypoint position, can start now
                self.position_map = self.map_name
                self.position = Vec2(self.last_keypoint.x, self.last_keypoint.y)
                self.moving_to_keypoint = false
            end
        end
        self:set_keypoint(best_keypoint, false)
        if self.last_keypoint.status == 'pending' then
            if self:move_to_offmap_keypoint(current_time) then
                return
            end
        elseif self.last_keypoint.status == 'completed' then
            return
        end
        self:process_offmap_command(current_time)
    end
end

local function read_npcs(filename)
    local result = {}
    local filename = game:get_config('game.npcs-file')
    if filename ~= '' then
        local npcs_data = dofile(filename)
        for name, data in pairs(npcs_data) do
            result[name] = NPC(name, data)
        end
    end
    return result
end

npcs = read_npcs()

function get_map_npcs()
    local map_npcs = {}
    for name, npc in pairs(npcs) do
        if npc.map_name == current_map.filename then
            table.insert(map_npcs, npc)
        end
    end
    return map_npcs
end

while true do
    for name, npc in pairs(npcs) do
        npc:update()
    end
    wait(0)
end
