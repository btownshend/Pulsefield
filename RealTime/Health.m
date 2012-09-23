classdef Health <handle
  properties
    remotestatus=struct('id',{},'lastmsgtime',{},'led',{});
    lastphase=0;
    maxwait=2;   % Max time to wait for an OK
  end
  methods
    function obj=Health(ids)
      for i=1:length(ids)
        % Fake initial ok message so they will be tracked
        obj.remotestatus(end+1)=struct('id',ids{i},'lastmsgtime',nan,'led','?');
      end
    end
    
    function gotmsg(obj,id)
    % fprintf('Health: got messages from %s\n', id);
      index=find(strcmp({obj.remotestatus.id},id),1);
      if isempty(index)
        fprintf('Added %s to health monitoring\n', id);
        obj.remotestatus(end+1)=struct('id',id,'lastmsgtime',now,'led','?');
      else
        obj.remotestatus(index).lastmsgtime=now;
      end
    end
    
    function print(obj)
      fprintf('   ID  Secs LED\n');
      for i=1:length(obj.remotestatus)
        fprintf('%5s %5.1f %d\n',obj.remotestatus(i).id, (now-obj.remotestatus(i).lastmsgtime)*24*3600, obj.remotestatus(i).led);
      end
    end
    
    function updateleds(obj)
    % Check uptimes
      phase=mod(floor(now*24*3600*4),4)<3;   % on-3s, off-1s
      for i=1:length(obj.remotestatus)
        rs=obj.remotestatus(i);
        silence=(now-obj.remotestatus(i).lastmsgtime)*24*3600;
        if ~isfinite(rs.lastmsgtime)
          newcolor='yellow';
        elseif silence>obj.maxwait
          newcolor='red';
        else
          newcolor='green';
        end
        
        if ~strcmp(obj.remotestatus(i).led,newcolor)
          oscmsgout('TO',sprintf('/health/%s/color',rs.id),{newcolor});
          obj.remotestatus(i).led=newcolor;
        end

        % Flash LED to indicate it is being monitored
        if phase~=obj.lastphase
          oscmsgout('TO',sprintf('/health/%s',rs.id),{int32(phase)});
        end
      end
      obj.lastphase=phase;
    end
  end
end
