function arduino_port=arduino_ip(dosync)
if nargin<1
  dosync=1;
end
arduino_port=[];
global arduino_ip_port;
if isempty(arduino_ip_port)
  fprintf('Opening arduino IP port\n');
  try 
    arduino_ip_port = jtcp('REQUEST','192.168.0.154',1500,'SERIALIZE',false);
  catch me
    error('Failed open of Arduino ethernet port: %s',me.message);
  end
end
if dosync
  for i=1:4
    syncok=0;
    try 
      syncok=sync(arduino_ip_port);
    catch me
      fprintf('Caught sync exception\n');
    end
    if syncok
      break;
    else
      fprintf('Reopening port\n');
      try
        jtcp('CLOSE',arduino_ip_port);
        arduino_ip_port = jtcp('REQUEST','192.168.0.154',1500,'SERIALIZE',false);
      catch
        fprintf('Failed open of port\n');
      end
    end
  end
  if ~syncok
    error('Unable to sync with Arduino');
  end
end
arduino_port=arduino_ip_port;
