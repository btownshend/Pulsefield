% Close arduino port
function arduino_close
global arduino_ip_port;
if isempty(arduino_ip_port)
  fprintf('Arduino already closed\n');
else
  try
    jtcp('CLOSE',arduino_ip_port);
  catch me
    error('Failed close of port: %s\n',me.message);
  end
  clear global arduino_ip_port
end
