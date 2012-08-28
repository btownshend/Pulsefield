% Class to map ids to a unique channel
% Uses round-robin allocation
classdef ChannelMap < handle
  properties
    ids;	% Ids
    channels;   % Corresponding channels
    nchannels;
    nextchannel=1;
  end
  methods
    function obj=ChannelMap(nchannels)
      obj.channels=[];
      obj.ids=[];
      obj.nchannels=nchannels;
      obj.nextchannel=1;
    end
    
    function setnumchannels(obj,nchannels)
      if nchannels==obj.nchannels
        return;
      end
      newmap=ChannelMap(nchannels);
      for i=1:length(obj.ids)
          newmap.newchannel(obj.ids(i))
      end
      obj.ids=newmap.ids;
      obj.channels=newmap.channels;
      obj.nchannels=newmap.nchannels;
      obj.nextchannel=newmap.nextchannel;
    end
      
    function channel=id2channel(obj,id)
      pos=find(obj.ids==id);
      if ~isempty(pos)
        channel=obj.channels(pos);
      else
        fprintf('Error trying to locate channel for id %d, ids=%s channels=%s\n', id, sprintf('%d ',obj.ids),sprintf('%d ',obj.channels));
        channel=[];
      end
    end
    
    function channel=newchannel(obj,id)
      if any(obj.ids==id)
        channel=obj.channels(find(obj.ids==id,1));
        fprintf('channelmap::newchannel - id %d already allocated to channel %d\n', id, channel);
        return;
      end

      for maxuse=1:20
        for i=1:obj.nchannels
          % Round-robin channel allocation
          c=mod((i-2)+obj.nextchannel,obj.nchannels)+1;
          used=sum(obj.channels==c);
          if used<maxuse
            fprintf('Allocating channel %d to id %d with usage %d\n', c, id, used);
            obj.channels(end+1)=c;
            obj.ids(end+1)=id;
            obj.nextchannel=mod(c,obj.nchannels)+1;
            channel=c;
            return;
          end
        end
      end
      fprintf('No channels available to allocate to id %d\n',id);
      channel=[];
    end

    function deleteid(obj,id)
      sel=obj.ids~=id;
      obj.channels=obj.channels(sel);
      obj.ids=obj.ids(sel);
    end
    
    function id=channel2id(obj,channel)
      id=obj.ids(obj.channels==channel);
    end

    function n=numchannels(obj)
      n=obj.nchannels;
    end
  end
end
