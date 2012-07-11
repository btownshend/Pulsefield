function arduino_port=arduino_start(dosync)
if nargin<1
  dosync=1;
end
global arduino_port;
if isempty(arduino_port) || ~strcmp(arduino_port.Status,'open')
  fprintf('Opening arduino port\n');
  arduino_port = serial('/dev/tty.usbmodem3d171','DataTerminalReady','off','OutputBufferSize',16384,'Timeout',1);    % define serial port
  arduino_port.BaudRate=115200;               % define baud rate
  set(arduino_port, 'terminator', 'LF');    % define the terminator for println
  try
    fopen(arduino_port);
  catch ex
    openfd=instrfind('Status','open')
    if length(openfd)>=1
      fclose(openfd(1));
    else
      error(ex.message);
    end
    fopen(arduino_port);
  end
  pause(4);
end
if dosync && ~sync(arduino_port)
  fprintf('Retrying sync\n');
  if ~sync(arduino_port)
    error('Unable to sync with Arduino');
  end
end
fprintf('Arduino ready\n');

