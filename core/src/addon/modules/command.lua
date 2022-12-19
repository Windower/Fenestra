--[[
Copyright © Windower Dev Team

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation files
(the "Software"),to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge,
publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
]]

-- LuaFormatter off
local -- params
    registry,
    call_command_handler_key,
    native_register_handler,
    native_unregister_handler,
    native_parse_args,
    native_input_ptr = ...
-- LuaFormatter on

local ffi = require('ffi')
local string = require('string')
local table = require('table')

local event = require('core.event')
local serializer = require('core.serializer')

local assert = assert
local error = error
local pairs = pairs
local select = select
local tonumber = tonumber
local tostring = tostring
local type = type
local unpack = unpack

local string_find = string.find
local string_gsub = string.gsub
local string_match = string.match
local string_sub = string.sub

local table_concat = table.concat

local event_trigger = event.trigger

-- Core API

local source_map = {
    ['console'] = -1,
    [-1] = 'console',
    ['client'] = 0,
    [0] = 'client',
    ['user'] = 1,
    [1] = 'user',
    ['macro'] = 2,
    [2] = 'macro',
    ['sub_target'] = 3,
    [3] = 'sub_target',
    ['sub_target_pc'] = 4,
    [4] = 'sub_target_pc',
    ['sub_target_npc'] = 5,
    [5] = 'sub_target_npc',
    ['sub_target_party'] = 6,
    [6] = 'sub_target_party',
    ['sub_target_alliance'] = 7,
    [7] = 'sub_target_alliance'
}

local register_handler
local unregister_handler
local call_command_handler
local unknown_command
do
    local handlers = {}

    register_handler = function(command, handler, raw)
        if type(command) ~= 'string' then
            error('bad argument #1 to \'register\' (string expected, got ' ..
                      type(command) .. ')')
        end

        if type(handler) ~= 'function' then
            error('bad argument #2 to \'register\' (function expected, got ' ..
                      type(handler) .. ')')
        end

        if raw == nil then
            raw = false
        elseif type(raw) ~= 'boolean' then
            error('bad argument #3 to \'register\' (boolean expected, got ' ..
                      type(raw) .. ')')
        end

        handlers[command] = handler
        native_register_handler(command, raw)
    end

    unregister_handler = function(command)
        if type(command) ~= 'string' then
            error('bad argument #1 to \'unregister\' (string expected, got ' ..
                      type(command) .. ')')
        end
        native_unregister_handler(command)
        handlers[command] = nil
    end

    call_command_handler = function(command, source, ...)
        if command == nil then
            local command_text, handled_value = ...
            local handled = {set = handled_value}
            event_trigger(unknown_command, source, command_text, handled)
            return not not handled.set
        else
            local handler = handlers[command]
            if not handler then error('handler not found') end
            handler(source_map[source], ...)
        end
    end
end

local parse_args = function(arg_string, arg_count)
    if type(arg_string) ~= 'string' then
        error('bad argument #1 to \'parse_args\' (string expected, got ' ..
                  type(arg_string) .. ')')
    end

    if arg_count == nil then
        arg_count = -1
    elseif type(arg_count) ~= 'number' then
        error('bad argument #2 to \'parse_args\' (number expected, got ' ..
                  type(arg_count) .. ')')
    elseif arg_count < 0 then
        error('bad argument #2 to \'parse_args\' (' ..
                  'expected a non-negative integer, got ' .. arg_count .. ')')
    elseif arg_count >= 2 ^ 32 - 1 then
        error('bad argument #2 to \'parse_args\' (' ..
                  'expected an integer less than 4294967295, got ' .. arg_count ..
                  ')')
    elseif (arg_count + 2 ^ 52) - 2 ^ 52 ~= arg_count then
        error('bad argument #2 to \'parse_args\' (' ..
                  'expected an integer, got ' .. arg_count .. ')')
    end

    return native_parse_args(arg_string, arg_count)
end

unknown_command = event.new()

rawset(registry, call_command_handler_key, call_command_handler)

local command_core = {
    register = register_handler,
    unregister = unregister_handler,
    parse_args = parse_args,
    unknown_command = unknown_command
}

-- High-level API

local arg_types = {}
local arg = setmetatable({}, {__index = arg_types})

local stored_arguments = {}

-- Helper and validation functions

local type_check = function(value, expected, name, index)
    local got = type(value)
    if type(expected) == 'table' then
        for i = 1, #expected do if got == expected[i] then return end end

        error(
            'bad argument #' .. tostring(index) .. ' to \'' .. name .. '\' (' ..
                table_concat(expected, ' or ') .. ' expected, got ' .. got ..
                ')')
    else
        if got ~= expected then
            error('bad argument #' .. tostring(index) .. ' to \'' .. name ..
                      '\' (' .. expected .. ' expected, got ' .. got .. ')')
        end
    end
end

local dump_value
dump_value = function(value)
    if type(value) == 'string' then return '\'' .. value .. '\'' end

    if type(value) == 'table' then
        local dumped = {}
        for key, inner in ipairs(value) do
            dumped[key] = dump_value(inner)
        end

        return '{' .. table_concat(dumped, ',') .. '}'
    end

    return tostring(value)
end

local var_info = function(var) return type(var) .. ': ' .. dump_value(var) end

local check_argument_parameter = function(command, index, argument, field,
                                          expected)
    if argument[field] == nil or type(argument[field]) == expected then
        return
    end

    local prefix
    if command and index then
        prefix = 'Command ' .. command.name .. ', argument #' .. index
        if argument.name then
            prefix = prefix .. ' (' .. argument.name .. ')'
        end
    else
        prefix = 'Argument ' .. argument.name
    end

    error(prefix .. ': Parameter \'' .. field .. '\' expected to be ' ..
              expected .. ', got ' .. var_info(argument[field]))
end

local validate_argument = function(argument, command, index)
    check_argument_parameter(command, index, argument, 'required', 'boolean')
    check_argument_parameter(command, index, argument, 'type', 'table')
    if argument.count ~= '*' then
        check_argument_parameter(command, index, argument, 'count', 'number')
    end
    check_argument_parameter(command, index, argument, 'options', 'table')
end

local validate_arguments = function(arguments, name)
    local required = true
    for _, argument in ipairs(arguments) do
        if not argument.required then
            required = false
        elseif not required then
            error('Command ' .. name ..
                      ' cannot have required arguments following optional ' ..
                      'arguments.')
        end
    end
end

arg.register_type = function(name, arg_type)
    type_check(name, 'string', 'register_type', 1)
    type_check(arg_type, 'table', 'register_type', 2)

    arg_type.name = name

    arg_types[name] = arg_type
end

local prepare_args = function(...)
    local arguments = {}
    local arg_index = 0
    for i = 1, select('#', ...) do
        local argument = select(i, ...)
        type_check(argument, {'table', 'string'}, 'register', 2)

        if type(argument) == 'table' then
            arg_index = arg_index + 1
            arguments[arg_index] = argument
        else
            local parsed = arg.parse(argument)
            for j = 1, #parsed do
                arg_index = arg_index + 1
                arguments[arg_index] = parsed[j]
            end
        end
    end

    return arguments
end

arg.register = function(name, ...)
    type_check(name, 'string', 'register', 1)

    stored_arguments[name] = prepare_args(...)
end

-- Command handling

local new
local delete
do
    local check_nested
    check_nested = function(t, ...)
        if select('#', ...) == 0 then return t end

        local nested = t.nested
        if not nested then return nil end

        local child = nested[...]
        if not child then return nil end

        return check_nested(child, select(2, ...))
    end

    local register_command = function(prefix, stored_commands, use_source, ...)
        local fn_index
        local argc = select('#', ...)

        local store = stored_commands
        local name = '/' .. prefix
        for i = 1, argc do
            local value = select(i, ...)
            if type(value) == 'function' then
                fn_index = i
                break
            end

            type_check(value, 'string', 'register', i + 1)
            assert(not store.command,
                   'Cannot register a more specific command under general ' ..
                       'command \'' .. name .. '\'.')

            name = name .. ' ' .. value

            local nested = store.nested
            if not nested then
                nested = {}
                store.nested = nested
            end

            local child = nested[value]
            if not child then
                child = {parent = store}
                nested[value] = child
            end

            store = child
        end

        local action = select(fn_index or argc + 1, ...)
        type_check(action, 'function', 'register', argc + 2)

        local arguments = prepare_args(select(fn_index + 1, ...))

        assert(not store.commands,
               'Command already registered for \'' .. name .. '\'.')
        assert(not store.nested, 'Cannot register command \'' .. name ..
                   '\' because more specific commands are already registered.')

        local command = {
            name = name,
            arguments = arguments,
            action = action,
            use_source = use_source
        }

        for index, argument in ipairs(arguments) do
            if argument.required == nil then
                argument.required = argument.default == nil
            end

            validate_argument(argument, command, index)
        end

        validate_arguments(command.arguments, name)

        store.command = command
    end

    local filter_error =
        function(message) return message:match(':%d+: (.*)') end

    local argument_error = function(command, argument, index, message)
        local prefix = 'Command \'' .. command .. '\', argument #' .. index
        if argument.name then
            prefix = prefix .. ' (' .. argument.name .. ')'
        end

        error(prefix .. ': ' .. message)
    end

    local call_handler = function(command, source, ...)
        if command.use_source then
            command.action(source, ...)
        else
            command.action(...)
        end
    end

    local command_cache = {}

    local prepare_args = function(command, source, ...)
        local argc = select('#', ...)
        local processed = 0
        local parsed_args = {}
        local parsed_count = 0
        for i = 1, #command.arguments do
            local argument = command.arguments[i]

            local argument_type = argument.type

            local variant = argument.count == '*' or argument_type and
                                argument_type.count == '*'
            if variant and argument.required and argc <= processed then
                argument_error(command.name, argument, i,
                               'Required at least one, but none was provided.')
            end

            local count = variant and argc - processed or argument.count or 1

            local block = {}
            for i = 1, count do
                processed = processed + 1
                local current = select(processed, ...)

                if not current and argument.required then
                    argument_error(command.name, argument, i,
                                   'Required but was not provided.')
                end

                if current and argument_type and argument_type.check then
                    local status, value =
                        pcall(argument_type.check, current,
                              argument.options or {})
                    if not status then
                        argument_error(command.name, argument, i,
                                       filter_error(value))
                    end
                    current = value
                end

                if current == nil then current = argument.default end

                block[i] = current
            end

            if count > 0 and argument_type and argument_type.pack then
                parsed_count = parsed_count + 1
                parsed_args[parsed_count] = argument_type.pack(block)
            else
                for i = 1, count do
                    parsed_count = parsed_count + 1
                    parsed_args[parsed_count] = block[i]
                end
            end
        end

        assert(processed >= argc,
               'Too many arguments for command ' .. command.name ..
                   '. Processed ' .. tostring(processed) ..
                   ' arguments, remainder: "' ..
                   table.concat({...}, ' ', processed + 1) .. '".')

        return parsed_args
    end

    local get_syntax_for_argument = function(argument, index)
        local argument_name = argument.name
        local token = ''

        local argument_type = argument.type
        if argument_name then token = argument_name .. ': ' end

        local generate_name = argument_type.generate_name
        local argument_options = argument.options or {}
        token = token ..
                    (generate_name and generate_name(argument_options) or
                        argument_type.name)

        local generate_args = argument_type.generate_args
        local check_args = generate_args and generate_args(argument_options) or
                               ''
        if check_args ~= '' then
            token = token .. '(' .. check_args .. ')'
        end

        local argument_default = argument.default
        if argument_default ~= nil then
            token = token .. ' = ' .. tostring(argument_default)
        end

        local enclosed
        if argument.required then
            enclosed = '<' .. token .. '>'
        else
            enclosed = '[' .. token .. ']'
        end

        return argument.count and (enclosed .. argument.count) or enclosed
    end

    local get_syntax_for_command = function(command)
        local tokens = {command.name}

        local arguments = command.arguments
        for i = 1, #arguments do
            tokens[i + 1] = get_syntax_for_argument(arguments[i], i)
        end

        return table.concat(tokens, ' ')
    end

    local build_syntax
    build_syntax = function(path, command)
        assert(command and (command.nested or command.command),
               'No commands registered for path \'' .. path .. '\'.')

        if command.command then
            return get_syntax_for_command(command.command)
        end

        local keys = {}

        for key in pairs(command.nested) do keys[#keys + 1] = key end

        table.sort(keys)

        local lines = {}
        local count = 0
        for _, key in ipairs(keys) do
            local nested = build_syntax(path .. ' ' .. key, command.nested[key])
            count = count + 1
            lines[count] = nested
        end

        return table.concat(lines, '\n')
    end

    new = function(name)
        local stored_commands = {}

        local root_command = {}
        root_command.register = function(this, ...)
            assert(this == root_command)
            register_command(name, stored_commands, false, ...)
        end
        root_command.register_source = function(this, ...)
            assert(this == root_command)
            register_command(name, stored_commands, true, ...)
        end
        root_command.unregister = function(this, ...)
            assert(this == root_command)
            local store = check_nested(stored_commands, ...)
            assert(store and store.command, 'Command \'' ..
                       table.concat({...}, ' ') .. '\' not registered.')

            store.command = nil
            local parent = store
            for i = select('#', ...), 1, -1 do
                parent = parent.parent
                parent.nested[select(i, ...)] = nil
                if next(parent.nested) then break end
            end
        end
        root_command.syntax = function(this, ...)
            assert(this == root_command)
            local store = check_nested(stored_commands, ...)
            local path = '/' .. name
            if ... then
                path = path .. ' ' .. table.concat({...}, ' ')
            end
            assert(store, 'No commands registered for \'' .. path .. '\'.')

            return table.concat(build_syntax(path, store), '\n')
        end

        command_cache[root_command] = {
            name = name,
            stored_commands = stored_commands
        }

        command_core.register(name, function(source, ...)
            local store = stored_commands
            local index = 0

            while store and not store.command and store.nested do
                index = index + 1
                store = store.nested[select(index, ...)]
            end
            if not store or not store.command then
                print('Invalid command for \'/' .. name .. '\'.')
                print(build_syntax('/' .. name, stored_commands))
                return
            end

            local status, result = pcall(prepare_args, store.command, source,
                                         select(index + 1, ...))
            if not status then
                print('Invalid command syntax for \'/' .. name .. '\': ' ..
                          result)
                local path = '/' .. name
                if index > 0 then
                    path = path .. ' ' .. table.concat({...}, 1, index)
                end
                print(build_syntax(path, store))
                return
            end

            call_handler(store.command, source, unpack(result))
        end)

        return root_command
    end

    delete = function(root_command)
        command_core.unregister(command_cache[root_command].name)
        command_cache[root_command] = nil
    end
end

-- Argument checking functions

do
    arg_types.string = {
        name = 'string',
        parse_options = function(match) return {match = match} end,
        check = function(str, options)
            assert(not options.match or string_match(str, options.match),
                   'Invalid pattern.')

            return str
        end,
        generate_args = function(options)
            return table_concat({options.match}, ',')
        end
    }
    arg_types.text = {
        name = 'text',
        count = '*',
        pack = function(block) return table_concat(block, ' ') end
    }
    arg_types.number = {
        name = 'number',
        parse_options = function(min, max)
            return {min = tonumber(min), max = tonumber(max)}
        end,
        check = function(str, options)
            local num = tonumber(str)

            assert(num, 'Number expected. Got: ' .. var_info(str))
            assert(not options.min or num >= options.min, 'Minimum ' ..
                       tostring(options.min) .. '. Got: ' .. tostring(num))
            assert(not options.max or num <= options.max, 'Maximum ' ..
                       tostring(options.max) .. '. Got: ' .. tostring(num))

            return num
        end,
        generate_args = function(options)
            return table_concat({options.min, options.max}, ',')
        end
    }
    arg_types.integer = {
        name = 'integer',
        parse_options = function(min, max)
            return {min = tonumber(min), max = tonumber(max)}
        end,
        check = function(str, options)
            local num = arg_types.number.check(str, options)

            assert(num % 1 == 0, 'Whole number expected. Got: ' .. tostring(num))

            return num
        end,
        generate_args = function(options)
            return table_concat({options.min, options.max}, ',')
        end
    }
    arg_types.one_of = {
        name = 'one_of',
        parse_options = function(...) return {...} end,
        check = function(str, options)
            for _, option in ipairs(options) do
                if option == str then return str end
            end

            error('Value needs to be one of: ' .. dump_value(options) ..
                      '. Got: ' .. dump_value(str))
        end,
        generate_args = function(options)
            return table_concat(options, ',')
        end
    }
end

-- Argument parsing from syntax

do
    local deep_copy
    deep_copy = function(t)
        local res = {}
        for key, value in pairs(t) do
            res[key] = type(value) == 'table' and deep_copy(value) or value
        end
        return res
    end

    local retrieve_argument = function(name)
        type_check(name, 'string', 'get', 1)

        local found_argument = stored_arguments[name]
        assert(found_argument, 'No argument named \'' .. name .. '\' found.')

        return deep_copy(found_argument)
    end

    local brace_match = {['<'] = '>', ['['] = ']', ['{'] = '}'}

    local parse_type = function(type_str)
        local _, name_end, name = string_find(type_str, '(.-)%(')
        if not name_end then name = type_str end

        local arg_type = arg_types[name]
        assert(arg_type ~= nil, 'Unknown argument function: ' .. type_str)

        if not name_end then return arg_type end

        assert(string_sub(type_str, #type_str, #type_str) == ')',
               'Malformed argument function call, expected \')\' to close \'(\': ' ..
                   type_str)

        local arg_string = string_sub(type_str, name_end) or ''
        local args = {}
        local arg_count = 0
        local index = 1
        while index ~= nil and index < #arg_string do
            local _, match_end, match = string_find(arg_string, '(.-)[,%)]',
                                                    index + 1)
            arg_count = arg_count + 1
            args[arg_count] = match
            index = match_end
        end

        return arg_type, arg_type.parse_options(unpack(args))
    end

    local parse_single = function(token, open)
        local argument = {}

        local _, name_end, name = string_find(token, '(.-):')
        argument.name = name

        local _, type_end, type = string_find(token, '(.-)=',
                                              name and (name_end + 1))
        if not type_end then
            local arg_type, options = parse_type(name and
                                                     string_sub(token,
                                                                name_end + 1) or
                                                     token)
            argument.type = arg_type
            argument.options = options
            return argument
        end

        assert(open ~= '<', 'Required arguments cannot have default values.')

        local arg_type, options = parse_type(type)
        argument.type = arg_type
        argument.options = options

        local default = string_match(token, '(.-)$', type_end + 1)
        if default then
            local check = arg_type.check
            argument.default = check and check(default, options or {}) or
                                   default
        end

        return argument
    end

    local find_first = function(str, compare, index)
        local escape = false
        local length = #str
        for i = index, length do
            if escape then
                escape = false
            else
                local char = string_sub(str, i, i)
                if char == '\\' then
                    escape = true
                elseif char == compare then
                    return string_gsub(string_sub(str, index, i - 1), '\\(.)',
                                       '%1'), i
                end
            end
        end

        return nil, nil
    end

    arg.parse = function(str)
        type_check(str, 'string', 'parse', 1)

        local arguments = {}

        local index = 1
        while index <= #str do
            index = string_find(str, '[^ ]', index)

            local open = string_sub(str, index, index)
            local close = brace_match[open]
            local match, close_index = find_first(str, close, index + 1)
            assert(match,
                   'Invalid argument string, expected \'' .. close ..
                       '\' to close \'' .. open .. '\' at position ' .. index ..
                       '.')
            local _, last_index, count = string_find('^%d+', close_index + 1)

            if not last_index then
                if string_sub(str, close_index + 1, close_index + 1) == '*' then
                    count = '*'
                    last_index = close_index + 1
                else
                    count = nil
                    last_index = close_index
                end
            else
                count = tonumber(count)
            end

            if open == '{' then
                local ref = retrieve_argument(match)
                assert(ref, 'Argument reference \'' .. match .. '\' not found.')

                local ref_count = #ref
                local arg_index = #arguments
                if count == '*' then
                    -- TODO: Implement...
                    assert(ref_count == 1, 'Variable argument count on ' ..
                               'multiple arguments not yet implemented.')

                    local argument = ref[1]
                    argument.count = '*'
                    arguments[arg_index + 1] = argument
                else
                    for _ = 1, count or 1 do
                        for i = 1, ref_count do
                            arg_index = arg_index + 1
                            arguments[arg_index] = ref[i]
                        end
                    end
                end
            else
                local argument = parse_single(match, open)
                argument.required = open == '<'

                argument.count = count

                validate_argument(argument)

                arguments[#arguments + 1] = argument
            end

            index = last_index + 1
        end

        return arguments
    end
end

local input
do
    local native_input = ffi.new('void(*)(char const*, size_t, int32_t)',
                                 native_input_ptr)
    input = function(command, source)
        if type(command) ~= 'string' then
            error('bad argument #1 to \'input\' (string expected, got ' ..
                      type(command) .. ')')
        end

        local native_source = source_map[source]
        if source == nil then
            native_source = 1
        elseif type(source) ~= 'string' then
            error('bad argument #2 to \'input\' (string expected, got ' ..
                      type(source) .. ')')
        else
            native_source = source_map[source]
            if not native_source then
                error(
                    'bad argument #2 to \'input\' (unknown command source \'' ..
                        source .. '\')')
            end
        end

        native_input(command, #command, native_source)
    end
end

local command = {
    core = command_core,
    arg = arg,
    new = new,
    delete = delete,
    input = input
}

serializer.register('__command', command, false)
serializer.register('__command.core', command_core, false)
serializer.register('__command.core.register', register_handler, false)
serializer.register('__command.core.unregister', unregister_handler, false)
serializer.register('__command.core.parse_args', parse_args, false)
serializer.register('__command.core.unknown_command', unknown_command, false)
serializer.register('__command.arg', arg, false)
serializer.register('__command.new', new, false)
serializer.register('__command.delete', delete, false)
serializer.register('__command.input', input, false)

return command
