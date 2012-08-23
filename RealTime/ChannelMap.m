% Class to map ids to a unique channel
% Uses round-robin allocation
classdef ChannelMap < handle
  properties
    channels=zeros(1,8);
    nextchannel=1;
  end
  methods
    function obj=ChannelMap(nchannels)
      obj.channels=zeros(1,nchannels);
      obj.nextchannel=1;
    end
    
    function channel=id2channel(obj,id)
      channel=find(obj.channels==id);
      if length(channel)~=1
        fprintf('Error trying to locate channel for id %d, channels=%s\n', id, sprintf('%d ',obj.channels));
        channel=[];
      end
    end
    
    function channel=newchannel(obj,id)
      if any(obj.channels==id)
        channel=find(obj.channels==id,1);
        fprintf('channelmap::newchannel - id %d already allocated to channel %d\n', id, channel);
        return;
      end

      for i=1:length(obj.channels)
        % Round-robin channel allocation
        c=mod((i-2)+obj.nextchannel,length(obj.channels))+1;
        if obj.channels(c)==0
          fprintf('Allocating channel %d to id %d\n', c, id);
          channel=c;
          obj.channels(c)=id;
          obj.nextchannel=mod(c,length(obj.channels))+1;
          return;
        end
      end
      fprintf('No channels available to allocate to id %d, channels=%s\n', id, sprintf('%d ',obj.channels));
    end

    function deleteid(obj,id)
      channel=id2channel(obj,id);
      if ~isempty(channel)
        obj.channels(channel)=0;
      end
    end
    
    function id=channel2id(obj,channel)
      id=obj.channels(channel);
    end

    function n=numchannels(obj)
      n=length(obj.channels);
    end
  end
end
