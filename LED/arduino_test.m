
col=zeros(699,3,'uint8');
s1=arduino_ip(1);
for i=1:100
  setallleds(s1,col,0);
end

zz

% Test arduino open capabilities
try
for i=1:10
  ap(i)=arduino_ip(1);
  clear global arduino_ip_port;
end
catch me
  fprintf('Failed after %d opens: %s\n', i-1, me.message);
  for j=1:i-1
    jtcp('CLOSE',ap(j));
  end
end