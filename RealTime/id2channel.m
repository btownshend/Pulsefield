% Map an ID back to a MIDI channel number
function channel=id2channel(info,id)
channel=find(info.channels==id);
if length(channel)~=1
  fprintf('Error trying to locate channel for id %d, channels=%s\n', id, shortlist(info.channels));
  channel=1;
end

