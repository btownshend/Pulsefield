% Class to map sets ids to a unique group id
% Uses round-robin allocation
classdef GroupMap < handle
  properties
    gids=[];	% Gids
    idsets={};   % Corresponding channels
    nextgid=1;
    maxgid=60;
  end
  methods
    function obj=GroupMap(maxgid)
      obj.maxgid=maxgid;
    end
    
    function idset=gid2idset(obj,gid)
      pos=find(obj.gids==gid);
      if ~isempty(pos)
        idset=obj.idsets{pos};
      else
        fprintf('Error trying to locate idset for gid %d, gids=%s\n', gid, sprintf('%d ',obj.gids));
        idset=[];
      end
    end
    
    function gid=idset2gid(obj,idset)
      if length(idset)<=1
        gid=0;   % Non-group
      end
      
      for i=1:length(obj.gids)
        if isempty(setxor(idset,obj.idsets{i}))
          gid=obj.gids(i);
          return;
        end
      end
      if length(obj.gids)==obj.maxgid
        fprintf('No more gids to allocate\n');
        gid=[];
        return;
      end
      obj.idsets{end+1}=idset;
      while ismember(obj.nextgid,obj.gids)
        obj.nextgid=mod(obj.nextgid,obj.maxgid)+1;
      end
      obj.gids(end+1)=obj.nextgid;
      obj.nextgid=mod(obj.nextgid,obj.maxgid)+1;
      gid=obj.gids(end);
    end
    
    function deletegid(obj,gid)
      sel=obj.gids~=gid;
      obj.idsets=obj.idsets(sel);
      obj.gids=obj.gids(sel);
    end
    
    function deleteuid(obj,uid)
    % Delete all groups containing the given uid
      sel=cellfun(@(z) ~ismember(uid,z), obj.idsets);
      obj.idsets=obj.idsets(sel);
      obj.gids=obj.gids(sel);
    end
    
    function n=numgid(obj)
      n=length(obj.gids)
    end

    function n=groupsize(obj,gid)
      pos=find(obj.gids==gid);
      if ~isempty(pos)
        n=length(obj.idsets{pos});
      else
        fprintf('groupsize: Unable to locate gid %d, gids=%s\n', gid, sprintf('%d ',obj.gids));
        n=0;
      end
    end
  end
end
