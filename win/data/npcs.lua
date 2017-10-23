return {
    ['NPC1'] = {
        sprite = 'data/sprite.spr',
        schedules = {
            first = {
                keypoints = {
                    {
                        map = 'data/test_tiled.tmx',
                        x = 70,
                        y = 25,
                        sequential = true,
                        day = 'odd',
                        timestamp = '0:0:3',
                        condition = function() return true end,
                        commands = {
                            {
                                type = 'move',
                                x = 0,
                                y = 200,
                            },
                            {
                                type = 'wait',
                                duration = 1,
                            },
                            {
                                type = 'text',
                                text = 'hello my friend',
                                duration = 3,
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
                                y = 200,
                                map = 'data/test_tiled2.tmx',
                            },
                        },
                    },
                    {
                        map = 'data/test_tiled2.tmx',
                        x = 0,
                        y = 200,
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