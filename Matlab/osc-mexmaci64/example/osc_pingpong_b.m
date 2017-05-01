
a = osc_new_address('127.0.0.1', 3334);
s = osc_new_server(3333);

for t = 1:1001
  m = osc_recv(s, 10.0);

  if length(m) > 0
    m = struct('path', '/pong', 'data', {{m{1}.data{1} + 1}});
    err = 0;
    while err ~= 1
      err = osc_send(a, m);
    end
  else
    disp('No packet for 10 seconds, quitting.');
    break;
  end
end

osc_free_server(s);
osc_free_address(a);
