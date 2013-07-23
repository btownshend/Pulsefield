% Control separate processes
% pgm - either 'frontend' or 'ledserver' or 'all'
% op - 'start', 'stop', 'quit'
function pgmctl(p,pgm,op,varargin)
if strcmp(pgm,'frontend')
  % Front end control
  if strcmp(op,'start')
    startfrontend(p);
    return;
  elseif strcmp(op,'stop')
    startfrontend(p);
    return;
  elseif strcmp(op,'quit')
    stopfrontend(p);
    return;
  end
elseif strcmp(pgm,'ledserver')
  % LED Server
  if strcmp(op,'start')
    save('p.mat','p');
    cmd=sprintf('/Applications/MATLAB_R2012b.app/bin/matlab -nosplash -r ''load p; ledserver(p)''&');
    [r,s]=system(cmd)
  elseif strcmp(op,'stop')
    fprintf('Unimplemented: %s\n', op);
  elseif strcmp(op,'quit')
    fprintf('Unimplemented: %s\n', op);
  else
    fprintf('Unknown op: %s\n', op);
  end
else
  fprintf('Unknown program to control: %s\n', pgm);
end