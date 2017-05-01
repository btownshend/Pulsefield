% osc function demo, client side

% an osc_address is a destination for packets -- IP or domain-name and port
a = osc_new_address('127.0.0.1', 7000)

% an osc message is a struct in matlab 
% there are basically two fields, path and data.
% data can contain floating point scalars, logical scalars,
% strings, etc.
m = struct( ...
        'path', '/foobar', ...
        'data', {{logical(0), int32(1), 3.14159, logical(1), 'hello world'}} ...
        )

% types are inferred automatically from the native matlab type
% or you can specify the type in the osc type tag, osc_send will
% attempt to convert the type automatically
% this message is equivalent to the above
m = struct( ...
        'path', '/foobar', ...
        'tt', 'ifLs', ...
        'data', {{1, 3.14159, 1, 'hello world'}} ...
        )


% send the message...  
osc_send(a, m)

% multiple messages can be sent at the same time (as a "#bundle")
% to do this we use a cell-array of messages
% the return value err == 0 if transmission failed (e.g. due to connection
% denied, etc)
err = osc_send(a, {m,m,m})

% when we are done with the address it can be freed
osc_free_address(a)

