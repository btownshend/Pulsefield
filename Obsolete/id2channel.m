% id2channel - Map an ID back to a MIDI channel number
% Usage: [channel,info]=id2channel(info,id,allocate)
% info - info struct (w/ channels, nextchannel fields)
% id - id to allocate (1..)
% allocate - true to allocate a new channel if not found in existing (default:false)
%    - if allocate is used, info return arg must be specified
function [channel,info]=id2channel(info,id,allocate)
if nargin<3
  allocate=false;  % Don't allocate if not found
end

% Allocate allowed
if allocate && nargout<2
  error('id2channel requires info output arg when allocating');
end

channel=find(info.channels==id);
if length(channel)~=1
  if allocate
    for i=1:length(info.channels)
      % Round-robin channel allocation
      c=mod((i-1)+info.nextchannel,length(info.channels))+1;
      if info.channels(c)==0
        fprintf('Allocating channel %d to id %d\n', c, id);
        channel=c;
        info.channels(c)=id;
        info.nextchannel=mod(c,length(info.channels))+1;
        return;
      end
    end
    fprintf('No channels available to allocate to id %d, channels=%s\n', id, sprintf('%d ',info.channels));
  else
    fprintf('Error trying to locate channel for id %d, channels=%s\n', id, sprintf('%d ',info.channels));
    channel=[];
  end
end

