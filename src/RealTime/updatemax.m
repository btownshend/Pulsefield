% Update MAX with a new set of locations for the targets
function updatemax(tpos,prevtpos)
if nargin<2
   % Clear all old values since we don't know the specific ones to replace
   sendtomax('/coll',{'clear'});
elseif size(prevtpos,1) > size(tpos,1)
   % Just clear the ones we are not updating
   for i=size(tpos,1)+1:size(prevtpos,1)
     sendtomax('/coll',{'remove',int32(i)});
   end
end
% Send new values
for i=1:size(tpos,1)
  sendtomax('/coll',{'symbol',int32(i),single(tpos(i,1)),single(tpos(i,2))});
end

% Update step sequencer
sendtomax('/seq',{'active',int32(size(tpos,1)>0)});
for i=1:size(tpos,1)
  sendtomax('/seq',{'pitch',int32(i),pos2pitch(tpos(i,1))});
end
sendtomax('/seq',{'loop',int32(1),int32(max(1,size(tpos,1)))});

% Map a position in meters to a MIDI pitch val
function pitch=pos2pitch(pos)
pitch=int32(60+pos*10);   % 10 notes/meter centered at middle-C
