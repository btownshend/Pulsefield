function awrite(s1,cmd)
if isfield(s1,'socket')
  % Ethernet
  jtcp('write',s1,uint8(cmd));
else
  % Serial
  while ~strcmp(s1.TransferStatus,'idle')
    pause(0.001);
  end
  fwrite(s1,cmd,'async');
end
%fprintf('Sent %d\n',length(cmd));

