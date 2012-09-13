% Class for interfacing with Ableton Live
classdef Ableton < handle
  properties
    clipnames={};
    clipcolors=[];
    tracknames={};
    numscenes=60;
    playing=nan;
    debug=false;
  end
  
  methods
    function obj=Ableton()
      obj.update();
    end

    function nm=getclipname(obj, track, clip)
      if ((track+1)>length(obj.tracknames)) || ((clip+1)>size(obj.clipnames,2))
        fprintf('Ableton:getclipname(obj,%d,%d) with only %d tracks, %d clips\n', track, clip, length(obj.tracknames), size(obj.clipnames,2));
        nm=[];
      else
        nm=obj.clipnames{track+1,clip+1};
      end
    end
    
    function nm=gettrackname(obj,track)
      if ((track+1)>length(obj.tracknames))
        fprintf('Ableton:gettrackname(obj,%d) with only %d tracks\n', track, length(obj.tracknames));
        nm=[];
      else
        nm=obj.tracknames{track+1};
      end
    end
      
    
    function n=numtracks(obj)
      n=length(obj.tracknames);
    end

    function n=numclips(obj)
      n=size(obj.clipnames,2);
    end
    
    function update(obj,timeout)
      if nargin<2
        timeout=0.1;
      end
      obj.refresh(0.0);  % Clean out any old ones and initialize ports
      oscmsgout('AL','/live/name/track',{});
      oscmsgout('AL','/live/scenes',{});   %TODO - this is not working, not seeing reply
      % Retrieve replies with a timeout
      obj.refresh(timeout);
      fprintf('Ableton:update:  Numtracks=%d, Numscenes=%d\n', obj.numtracks,obj.numscenes);
      for track=0:obj.numtracks-1
        for clip=0:obj.numscenes-1
          %fprintf('Clip(%d,%d)\n', track,clip);
          oscmsgout('AL','/live/name/clip',{int32(track),int32(clip)});
        end
        obj.refresh(0.0);
      end
    end

    function refresh(obj,timeout)
      if nargin<2
        timeout=0.0;
      end
      % Refresh AL data from running instance
      nmsg=0;
      while true
        m=oscmsgin('MPA',timeout);
        if isempty(m)
          break;
        end
        nmsg=nmsg+1;
        if obj.debug
          fprintf('Got %s\n',formatmsg(m.path,m.data));
        end
        if strcmp(m.path,'/live/play')
          obj.playing=m.data{1};
        elseif strcmp(m.path,'/live/track/info')
        elseif strcmp(m.path,'/live/clip/info')
        elseif strcmp(m.path,'/live/clip/position')
        elseif strcmp(m.path,'/live/name/return')
        elseif strcmp(m.path,'/live/name/track')
          track=m.data{1};
          nm=m.data{2};
          obj.tracknames{track+1}=nm;
        elseif strcmp(m.path,'/live/name/clip')
          %fprintf('Got reply: %s\n',formatmsg(m.path,m.data));
          track=m.data{1};
          clip=m.data{2};
          nm=m.data{3};
          color=m.data{4};
          obj.clipnames{track+1,clip+1}=nm;
          obj.clipcolors(track+1,clip+1)=color;
        elseif strcmp(m.path,'/live/arm')
        elseif strcmp(m.path,'/live/mute')
        elseif strcmp(m.path,'/live/solo')
        elseif strcmp(m.path,'/live/volume')
        elseif strcmp(m.path,'/live/pan')
        elseif strcmp(m.path,'/live/send')
        elseif strcmp(m.path,'/live/master/volume')
        elseif strcmp(m.path,'/live/master/pan')
        elseif strcmp(m.path,'/live/master/crossfader')
        elseif strcmp(m.path,'/live/return/mute')
        elseif strcmp(m.path,'/live/return/solo')
        elseif strcmp(m.path,'/live/return/volume')
        elseif strcmp(m.path,'/live/return/pan')
        elseif strcmp(m.path,'/live/return/send')
        elseif strcmp(m.path,'/live/overdub')
        elseif strcmp(m.path,'/live/tempo')
        elseif strcmp(m.path,'/live/scene')
        elseif strcmp(m.path,'/live/scenes')
          obj.numscenes=m.data{1};
          fprintf('Num scenes=%d\n', obj.numscenes);
        elseif strcmp(m.path,'/live/track')
        elseif strcmp(m.path,'/live/master/meter')
        elseif strcmp(m.path,'/live/return/meter')
        elseif strcmp(m.path,'/live/track/meter')
        elseif strcmp(m.path,'/live/device/param')
        elseif strcmp(m.path,'/live/return/device/param')
        elseif strcmp(m.path,'/live/master/device/param')
        elseif strcmp(m.path,'/live/device/selected')
        elseif strcmp(m.path,'/live/return/device/selected')
        elseif strcmp(m.path,'/live/master/device/selected')
        elseif strcmp(m.path,'/live/beat')
        elseif strcmp(m.path,'/live/name/trackblock')
        else
          fprintf('Unrecognized reply from AL: %s\n', formatmsg(m.path,m.data));
        end
      end
      if nmsg>0
        fprintf('Received %d messages from AL\n', nmsg);
      end
    end
  end
end



