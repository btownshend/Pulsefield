% Run through power settings
nled=[0,1,2,4,8,16,32,64,128,256,300];
while true
  lvl=[127,127,127];
  fprintf('Lighting first N leds with level %d,%d,%d\n', lvl);
  for i=1:length(nled)
    s1=arduino_ip(1);
    fprintf('Lighting %d LEDs...\n',nled(i));
    setled(s1,-1,[0,0,0],1,1);
    if nled(i)>0
      setled(s1,[0,nled(i)-1],lvl,1,1);
    end
    show(s1);
    sync(s1);
    input('Press return');
  end
  fprintf('Lighting all LEDS with increasing power\n');
  for i=[0,1,2,4,8,16,32,64,128,255]
    s1=arduino_ip(1);
    fprintf('Lighting LEDs at power level %d...\n',i);
    setled(s1,-1,i*[1,1,1],1,1);
    show(s1);
    sync(s1);
    input('Press return');
  end
    
end