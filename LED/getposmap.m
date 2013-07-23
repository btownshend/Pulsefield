% Get position map for LEDS
function posmap=getposmap(vgroup)
% Order currently blue, cyan, red, green, purple, yellow
%3,5,1,2,4,6

if vgroup==0
%  posmap=[800+(29:-1:0),640+(159:-1:0),320+(159:-1:0),480+(0:159),0:159,160+(0:28),800+(159:-1:30),160+(30:159)];
%  posmap=[800+(29:-1:0),640+(159:-1:0),320+(159:-1:0),480+(0:159),0:159,160+(0:28)];
%  posmap=[0+(29:-1:0),160+(159:-1:0),320+(159:-1:0),480+(0:159),640+(0:159),800+(0:28)];
posmap=[480+(159:-1:0), 320+(159:-1:0),800+(159:-1:0),640+(0:159)];
elseif vgroup==1
  posmap=0:160*6-1;
  %fprintf('Not mapping IDs\n');
elseif vgroup==2
  % Map index to left entry group
  posmap=159-numled(0)+1:159;
elseif vgroup==3
  % Map index to right entry group
  posmap=[800+(159-numled(0)+1:159)];
else
  error('Invalid vgroup: %d',vgroup);
end
