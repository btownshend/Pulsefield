function etherwrite(s1,cmd)
while ~strcmp(s1.TransferStatus,'idle')
  pause(0.01);
end
fwrite(s1,cmd,'async');
fprintf('Sent %d\n',length(cmd));
