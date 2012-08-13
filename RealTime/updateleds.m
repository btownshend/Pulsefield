% Update LEDS to display tracking of each hypo
function updateleds(p,snap)
period=10.0;  % Period of pulsing (in seconds)
maxlev=0.6;
minlev=0.1;
minradius=1;   % min radius where marker is on
maxleds=15;    % Number of LEDs in marker when close to edge
% Amplitude overall
phase=snap.when*3600*24*2*pi/period;
amp=minlev+(maxlev-minlev)*(sin(phase)+1)/2;
lev=(amp.^2*.97+0.03) * 127;   % Response is nonlinear (approx squared)
% DISABLE - update rate is too slow, makes it jumpy -- need separate process to implement these types
% LEDs can update about 35 times/second, so could make any transitions without visible jitter
lev=maxlev*127;

%fprintf('amp=%.2f,lev=%.1f ',amp,lev);

awidthmax=(2*pi*48/50)/sum(~p.layout.outsider) * maxleds;
meanradius=(max(p.layout.active(:,1))-min(p.layout.active(:,2)))/2;

% Send new values
s1=arduino_ip(0);
setled(s1,[0,numled()-1],round(lev*p.colors{1}),1); 
[~,ord]=sort([snap.hypo.id]);
for ii=1:length(ord)
  i=ord(ii);
  h=snap.hypo(i);
  pos=h.pos;
  % Angular width of person's marker
  awidth=awidthmax * min(1,(norm(pos)-minradius)/meanradius);

  % Color of marker lights (track people)
  col=id2color(h.id,p.colors)*127;
  if norm(pos)>0.5   % At least .5m away from enter
    angle=cart2pol(pos(:,1),pos(:,2));
    langle=cart2pol(p.layout.lpos(:,1),p.layout.lpos(:,2));
    indices = find(abs(langle-angle)<awidth/2 & ~p.layout.outsider);   % All LEDs inside active area, within awidth angle of person
    if ~isempty(indices)
      setled(s1,indices-1,col);
      fprintf('setled(%s,[%d,%d,%d])\n',shortlist(indices),col);
    end
  end
  % Visual feedback of how many people are inside
  setled(s1,i-1,col,1);
end
%show(s1,0.1);   % Show for at least 0.1s
show(s1);
