frame=1;
addpath ../Common
addpath ../RealTime
window=[-0.17 0.17 0 4.0];
dest={'LASER','REC'};
oscmsgout(dest,'/pf/set/minx',{window(1)});
oscmsgout(dest,'/pf/set/maxx',{window(2)});
oscmsgout(dest,'/pf/set/miny',{window(3)});
oscmsgout(dest,'/pf/set/maxy',{window(4)});
rate=10;	% Update rate
period=2;	% Period of pattern in seconds
cperiod=[4,5,6];
theta=0;	% Current theta
dtheta=2*pi/(period*rate);
col=[1,1,1];
while true
  oscmsgout(dest,'/pf/frame',{frame});
  frame=frame+1;
  theta=theta+dtheta;
  x=(window(2)-window(1))*sin(frame/rate*2*pi/period)/2/1.5+(window(2)+window(1))/2;
  rawc=sin(frame/rate*2*pi./cperiod)*.4+0.6;
  col=mappower(rawc/max(rawc));
  oscmsgout(dest,'/laser/bg/begin',{});
  oscmsgout(dest,'/laser/set/color',{col(1),col(2),col(3)});
  oscmsgout(dest,'/laser/line',{x,window(3),x,window(4)});
  oscmsgout(dest,'/laser/line',{x,window(4),x,window(3)});
  oscmsgout(dest,'/laser/bg/end',{});
  pause(1/rate);
end

  
  
  