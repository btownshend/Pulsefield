s1=arduino_ip;
sync(s1);
level=10;
full=[127,127,127];
nled=numled();
vg=1;
while true
  fprintf('On (1-Red, 2=Green, 3=Blue, 4=White, 5=Cyan (G+B), 6=Magenta (R+B))\n');
  setled(s1,[0,159],[level,0,0],1,vg);
  setled(s1,0,full,1,vg);
  setled(s1,160+[0,159],[0,level,0],1,vg);
  setled(s1,160,full,1,vg);
  setled(s1,320+[0,159],[0,0,level],1,vg);
  setled(s1,320,full,1,vg);
  setled(s1,480+[0,159],[level,level,level],1,vg);
  setled(s1,480,full,1,vg);
  setled(s1,640+[0,159],[0,level,level],1,vg);
  setled(s1,640,full,1,vg);
  setled(s1,800+[0,159],[level,0,level],1,vg);
  setled(s1,800,full,1,vg);
  show(s1);
  pause(1);
  fprintf('Off\n');
  setled(s1,-1,[0,0,0],1);
  show(s1);
  pause(1);
end
