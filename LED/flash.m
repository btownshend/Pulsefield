s1=arduino_ip;
sync(s1);
level=10;
full=[127,127,127];
nled=numled();
while true
  fprintf('On (1-Red, 2=Green, 3=Blue, 4=White)\n');
  setled(s1,[0,159],[level,0,0],1);
  setled(s1,0,full,1);
  setled(s1,160+[0,159],[0,level,0],1);
  setled(s1,160,full,1);
  setled(s1,320+[0,159],[0,0,level],1);
  setled(s1,320,full,1);
  setled(s1,480+[0,nled-481],[level,level,level],1);
  setled(s1,480,full,1);
  show(s1);
  pause(1);
  fprintf('Off\n');
  setled(s1,-1,[0,0,0],1);
  show(s1);
  pause(1);
end
