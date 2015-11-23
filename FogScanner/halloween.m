frame=1;
addpath ../Common
addpath ../RealTime
window=[-30 30 0 30];
dest={'LASER','REC'};
oscmsgout(dest,'/pf/set/minx',{window(1)});
oscmsgout(dest,'/pf/set/maxx',{window(2)});
oscmsgout(dest,'/pf/set/miny',{window(3)});
oscmsgout(dest,'/pf/set/maxy',{window(4)});
rate=10;	% Update rate
period=20;	% Period of pattern in seconds
cperiod=[4,5,6];
theta=0;	% Current theta
dtheta=2*pi/(period*rate);
col=[1,1,1];
while true
  oscmsgout(dest,'/pf/frame',{frame});
  frame=frame+1;
  theta=theta+dtheta;
  minx=-11.67;
  minx=-20;
  maxx=10.72;
  width=30;
  xpos=(sin(theta)+1)/2*(maxx-minx-width)+minx;
  miny=15.88;
  maxy=15.88;
  rawc=sin(frame/rate*2*pi./cperiod)*.4+0.6;
  col=mappower(rawc/max(rawc));
  col=[0,1,0];
  oscmsgout(dest,'/laser/bg/begin',{});
  oscmsgout(dest,'/laser/set/color',{col(1),col(2),col(3)});
  oscmsgout(dest,'/laser/line',{xpos,miny,xpos+width,maxy});
  oscmsgout(dest,'/laser/line',{xpos+width,maxy,xpos,miny});
  oscmsgout(dest,'/laser/bg/end',{});
  pause(1/rate);
end

  
  
  