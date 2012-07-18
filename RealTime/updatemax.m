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
if size(tpos,1)>0
  pitches=pos2pitch(tpos(:,1));
  disprange=[min([pitches-4;36]),max([pitches+4;86])];
  sendtomax('/seq',{'zoom',int32(disprange(1)),int32(disprange(2))});
  sendtomax('/seq',{'loop',int32(1),int32(max(1,size(tpos,1)))});
  for i=1:size(tpos,1)
    sendtomax('/seq',{'pitch',int32(i),pos2pitch(tpos(i,1))});
  end
  sendtomax('/seq',{'active',int32(1)});
else
  sendtomax('/seq',{'active',int32(0)});
end

% Map a position in meters to a MIDI pitch val
function pitch=pos2pitch(pos)
pitch=int32(60+pos*10);   % 10 notes/meter centered at middle-C
