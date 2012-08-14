function [counter,cmd]=syncsend(s1,execute)
global counter
if nargin<2
  execute=true;
end
if isempty(counter)
  counter=1;
end
counter=mod(counter+1,256);
cmd=zeros(1,2,'uint8');
cmd(1)='V';
cmd(2)=counter;
if execute
  awrite(s1,cmd);
end
