return {
    ['NPC1'] = {
        sprite = 'data/sprite.spr',
        schedules = {
            first = {
                activation_script = 'centered_text(100, "test"):wait()',
                keypoints = {
                    {
                        map = 'data/test_tiled.tmx',
                        x = 68,
                        y = 20,
                        sequential = true,
                        day = 'odd',
                        timestamp = '0:0:3',
                        condition = function() return true end,
                        commands = {
                            {
                                type = 'move',
                                x = 0,
                                y = 206,
                            },
                            {
                                type = 'wait',
                                duration = 2,
                            },
                            {
                                type = 'text',
                                text = 'hello my friend',
                                duration = 4,
                                condition = function()
                                    return current_map:get_object('jimbo')
                                end,
                            },
                            {
                                type = 'pose',
                                pose = 'Pose Test',
                            },
                            {
                                type = 'teleport',
                                x = 0,
                                y = 206,
                                map = 'data/test_tiled2.tmx',
                            },
                        },
                    },
                    {
                        map = 'data/test_tiled2.tmx',
                        x = 0,
                        y = 206,
                        sequential = true,
                        direction = RIGHT,
                    },
                    {
                        map = 'data/test_tiled2.tmx',
                        x = 136,
                        y = 64,
                        direction = DOWN,
                        commands = {
                            {
                                type = 'wait',
                                duration = 3,
                            },
                            {
                                type = 'face',
                                direction = LEFT,
                            },
                        },
                    },
                },
            },
        },
    },
}