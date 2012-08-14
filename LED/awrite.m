function awrite(s1,cmd)
maxwrite=1024;
if isfield(s1,'socket')
  % Ethernet
  for i=1:maxwrite:length(cmd)
    jtcp('write',s1,uint8(cmd(i:min(end,i+maxwrite-1))));
  end
else
  % Serial
  while ~strcmp(s1.TransferStatus,'idle')
    pause(0.001);
  end
  fwrite(s1,cmd,'async');
end
%fprintf('Sent %d\n',length(cmd));

