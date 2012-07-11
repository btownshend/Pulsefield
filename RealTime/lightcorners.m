% Turn on LEDs on each corner
function lightcorners(layout)
s1=arduino_ip();
setled(s1,[0,length(layout.ldir)-1],[0,10,0],1);
setled(s1,0,[127,0,0],1);
last=layout.ldir(1,:);
for i=2:length(layout.ldir)
  cur=layout.ldir(i,:);
  if any(last~=cur)
    setled(s1,[i-1,i]-1,[127,0,0],1);
  end
  last=cur;
end
setled(s1,length(layout.ldir)-1,[127,0,0],1);
show(s1);
fprintf('All corner lights should now be on\n');
input('Press return to continue:');
setled(s1,-1,[0,0,0],1);
show(s1);