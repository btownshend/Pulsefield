
a = osc_new_address('127.0.0.1', 3333);
s = osc_new_server(3334);

m = struct('path', '/ping', 'data', {{1}})

osc_send(a, m);

tic;

for t = 1:1000
  m = osc_recv(s, 10.0);

  if length(m) > 0
    m = struct('path', '/ping', 'data', {{m{1}.data{1} + 1}});
    err = 0;
    while err ~= 1
      err = osc_send(a, m);
    end
  else
    disp('No packet for 10 seconds, quitting.');
    break;
  end
end

dt = toc;

disp('Average time to send/recv roundtrip...')
disp(toc / 1000);

osc_free_server(s);
