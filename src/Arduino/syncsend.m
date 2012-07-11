function counter=syncsend(s1)
global counter
if isempty(counter)
  counter=1;
end
counter=mod(counter+1,256);
cmd=zeros(1,2,'uint8');
cmd(1)='V';
cmd(2)=counter;
awrite(s1,cmd);
