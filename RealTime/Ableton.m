% Class for interfacing with Ableton Live
% External interface has track, clip with 1-origin
classdef Ableton < handle
  properties
    clipnames={};
    clipcolors=[];
    allstatus=[];
    trackstatus=[];
    tracknames={};
    songs={};  % songtrack->track mappings
    songids={}; % IDS
    songnames={}; % Names
    songtempo=[];  % Tempo of each song in BPM
    numscenes=60;
    playing=nan;
    debug=false;
    pll=PLL();
    volume=0.5;
    oscdests={'TO'};
  end
  
  methods
    function obj=Ableton(filename)
    % Stop Ableton (so it doesn't keep sending meter or position messages that prevent flushing)
      obj.stop(); 

      if nargin>=1
        obj.load(filename); 
        obj.assignsongs();
      end

      % Make sure all Ableton channels are off
      obj.stop();
      obj.stopalltracks();
      obj.initgui();
    end
    
    function checksong(obj,song)
      if isempty(song) || song<1 || song>length(obj.songs)
        throw(MException('Ableton:checksong','Song value %d out of bounds',song));
      end
    end

    function checksongtrack(obj,song,songtrack)
      obj.checksong(song);
      if isempty(songtrack) || songtrack<1 || songtrack>length(obj.songs{song})
        throw(MException('Ableton:gettrack','Bad song,songtrack: (%d,%d)',song,songtrack));
      end
    end
    
    function checktrack(obj,track)
      if isempty(track) || track<1 || track>length(obj.tracknames)
        throw(MException('Ableton:checktrack','Track value %d out of bounds',track));
      end
    end

    function checkclip(obj,clip)
      if isempty(clip) || clip<1 || clip>size(obj.clipnames,2)
        throw(MException('Ableton:checkclip','Clip value %d out of bounds',clip));
      end
    end
          
    function [song,songtrack]=getsong(obj,track)
      for song=1:length(obj.songs)
        songtrack=find(obj.songs{song}==track,1);
        if ~isempty(songtrack)
          return;
        end
      end
      song=[]; songtrack=[];
    end
    
    function tempo=getsongtempo(obj,song)
      obj.checksong(song);
      tempo=obj.songtempo(song);
    end
    
    function tempo=gettempo(obj)
      tempo=obj.pll.bpm;
    end
    
    function volume=getvolume(obj)
      volume=obj.volume;
    end
    
    function name=getsongname(obj,song)
      obj.checksong(song);
      name=obj.songnames{song};
    end
    
    function nm=getclipname(obj, track, clip)
      obj.checktrack(track);
      obj.checkclip(clip);
      nm=obj.clipnames{track,clip};
    end
    
    function track=gettrack(obj,song,songtrack)
    % Map from (song,songtrack) to absolute track
      obj.checksongtrack(song,songtrack);
      track=obj.songs{song}(songtrack);
    end
    
    function nm=getsongclipname(obj,song,songtrack,clip)
      nm=obj.getclipname(obj.gettrack(song,songtrack),clip);
    end
    
    function val=haveclip(obj,song,songtrack,clip)
      val=~isempty(obj.getsongclipname(song,songtrack,clip));
    end
    
    function nm=gettrackname(obj,track)
      obj.checktrack(track);
      nm=obj.tracknames{track};
    end
    
    function nm=getsongtrackname(obj,song,songtrack)
      nm=obj.gettrackname(obj.gettrack(song,songtrack));
    end
    
    function n=numtracks(obj)
      n=length(obj.tracknames);
    end

    function track=findtrack(obj,name)
      track=find(strcmp(name,obj.tracknames));
    end
    
    function song=findsongid(obj,id)
      song=find(strcmp(id,obj.songids));
    end
    
    function n=numsongtracks(obj,song)
      obj.checksong(song);
      n=length(obj.songs{song});
    end

    function n=numclips(obj)
      n=size(obj.clipnames,2);
    end
    
    function n=numsongs(obj)
      n=length(obj.songs);
    end
    
    function update(obj,timeout,getclips)
      if nargin<2
        timeout=0.5;
      end
      if nargin<3
        getclips=1;
      end
      
      % Clean out any old ones and initialize ports
      obj.clipnames={};
      obj.clipcolors=[];
      obj.allstatus=[];
      obj.tracknames={};
      obj.songs={};  % Start/stop track
      obj.songnames={}; % Names
      obj.songtempo=[];  % Tempo of each song in BPM

      obj.refresh(0.0);  
      oscmsgout('AL','/live/name/track',{});
      oscmsgout('AL','/live/scenes',{});   %TODO - this is not working, not seeing reply
                                           % Retrieve replies with a timeout
      obj.refresh(timeout);
      fprintf('Ableton:update:  Numtracks=%d, Numscenes=%d\n', obj.numtracks,obj.numscenes);
      if obj.numtracks==0
        fprintf('Failed to get any tracks -- retry with longer timeout\n');
        obj.refresh(2.0);
        fprintf('Ableton:refresh2:  Numtracks=%d, Numscenes=%d\n', obj.numtracks,obj.numscenes);
      end
        
      if getclips
        fprintf('Loading track ');
        for track=1:obj.numtracks
          fprintf('%d..',track);
          for clip=1:obj.numscenes
            %fprintf('Clip(%d,%d)\n', track,clip);
            oscmsgout('AL','/live/name/clip',{int32(track-1),int32(clip-1)});
            obj.refresh(0.01);
          end
        end
        fprintf('done\n');
        obj.assignsongs();
      end
    end
    
    function save(obj,filename)
    % Save track/clip info in a file
      sobj=struct('clipnames',{obj.clipnames},'clipcolors',obj.clipcolors,'tracknames',{obj.tracknames},'numscenes',obj.numscenes);
      save(filename,'-struct','sobj');
    end

    function load(obj,filename)
      fprintf('Loading Ableton config from %s\n',filename);
      newobj=load(filename,'-mat');
      % Verify that they match the current tracknames
      obj.update(0.5,0);
      mismatch=0;
      if isempty(obj.tracknames)
        fprintf('No tracks retrieved from Ableton Live.\n');
        [alhost,~]=getsubsysaddr('AL');
        fprintf('- is AL running on host %s\n',alhost);
        fprintf('- is abletonRelay.sh running on host %s\n',alhost);
        [mpahost,mpaport]=getsubsysaddr('MPA');
        fprintf('- is abletonRelay.sh setup to forward OSC messages incoming on port 9001 to this host (%s) port %d?\n', mpahost, mpaport);
        fprintf('- is there a network connection between this host and %s?\n', alhost);
      elseif length(obj.tracknames)~=length(newobj.tracknames)
        fprintf('Ableton setup loaded from %s has %d tracks, but AL currently has %d tracks\n', filename,length(newobj.tracknames),length(obj.tracknames));
        mismatch=1;
      else
        for i=1:length(obj.tracknames)
          if ~strcmp(obj.tracknames{i},newobj.tracknames{i})
            fprintf('Ableton setup loaded from %s has %s as track %d, but AL currently has "%s" as this track\n', filename,newobj.tracknames{i},i,obj.tracknames{i});
            mismatch=1;
            break;
          end
        end
      end
      if mismatch
        fprintf('Ableton file is mismatched -- Use ableton_query.m to update file if GRID app tracks have changed\n');
        % obj.update;   % Update is too slow when retrieving clips to do here
      end
      fn=fieldnames(newobj);
      for i=1:length(fn)
        obj.(fn{i})=newobj.(fn{i});
      end
    end

    function csvwrite(obj,filename)
    % Save layout in CSV file
      fd=fopen(filename,'w');
      for track=1:size(obj.clipnames,1)
        fprintf(fd,'%s,',obj.tracknames{track});
      end
      fprintf(fd,'scene\n');
      for scene=1:obj.numscenes
        for track=1:size(obj.clipnames,1)
          fprintf(fd,'"%s",',obj.clipnames{track,scene});
        end
        fprintf(fd,'%d\n',scene);
      end
      for track=1:size(obj.clipnames,1)
        fprintf(fd,'%d,',track);
      end
      fclose(fd);
    end

    function initgui(obj)
      for songtrack=1:length(obj.songs)
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/scene',songtrack),{''});
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/clip',songtrack),{''});
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/track',songtrack),{''});
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/pos',songtrack),{''});
      end
    end

    function nmsg=refresh(obj,timeout)
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
          track=m.data{1}+1;
          clip=m.data{2}+1;
          status=m.data{3};   % 1=stopped, 2=playing, 3=triggered
          obj.trackstatus(track)=status;
          
          try 
            [song,songtrack]=obj.getsong(track);
          catch me
            % Already indicated error, ignore it
            song=[];
          end
          if ~isempty(song)
            obj.allstatus(songtrack,clip)=status;
            statuses={'zero','stopped','playing','triggered'};
            fprintf('Song %d, Songtrack %d, Clip %d %s\n',song, songtrack, clip,statuses{status+1});
            playingclip=find(obj.allstatus(songtrack,:)==2);
            triggeredclip=find(obj.allstatus(songtrack,:)==3);
            if isempty(playingclip)
              sc='';
              cl='';
              % Nothing playing, so clear any leftover triggered clips that never played
              if ~isempty(triggeredclip)
                fprintf('Clearing leftover triggered clips on songtrack %d: %s\n', songtrack, shortlist(triggeredclip));
                obj.allstatus(songtrack,triggeredclip)=1;
                triggerclip=[];
              end
            else
              sc=sprintf('%d ',playingclip);
              cl=obj.getclipname(track,playingclip(1));
            end
            if ~isempty(triggeredclip)
              sc=[sc,' -> ',sprintf('%d ',triggeredclip)];
              cl=[cl,' -> ',obj.getclipname(track,triggeredclip(1))];
            end
            oscmsgout(obj.oscdests,sprintf('/grid/table/%d/scene',songtrack),{sc});
            oscmsgout(obj.oscdests,sprintf('/grid/table/%d/clip',songtrack),{cl});
            if isempty(playingclip) && isempty(triggeredclip)
              oscmsgout(obj.oscdests,sprintf('/grid/table/%d/track',songtrack),{''});
              oscmsgout(obj.oscdests,sprintf('/grid/table/%d/pos',songtrack),{''});
            else
              oscmsgout(obj.oscdests,sprintf('/grid/table/%d/track',songtrack),{obj.gettrackname(track)});
            end
          end
        elseif strcmp(m.path,'/live/clip/position')
          track=m.data{1}+1;
          clip=m.data{2}+1;
          position=m.data{3};
          %length=m.data{4};
          loop_start=m.data{5};
          loop_end=m.data{6};
          try 
            [~,songtrack]=obj.getsong(track);
          catch me
            songtrack=[];
          end
          if ~isempty(songtrack)
            oscmsgout(obj.oscdests,sprintf('/grid/table/%d/pos',songtrack),{sprintf('[%.0f-%.0f]@%.0f',loop_start,loop_end,position)});
            obj.allstatus(songtrack,clip)=2;  % In case info was missed
          end
        elseif strcmp(m.path,'/live/name/return')
        elseif strcmp(m.path,'/live/name/track')
          track=m.data{1};
          nm=m.data{2};
          if ~isempty(track)
            obj.tracknames{track+1}=nm;
            obj.trackstatus(track+1)=1;   % Assume stopped
          end
        elseif strcmp(m.path,'/live/name/clip')
          %fprintf('Got reply: %s\n',formatmsg(m.path,m.data));
          track=m.data{1};
          clip=m.data{2};
          nm=m.data{3};
          color=m.data{4};
          if track<0 || clip<0
            fprintf('/live/name/clip: Bad track or clip (%d,%d)\n', track, clip);
          else
            obj.clipnames{track+1,clip+1}=nm;
            obj.clipcolors(track+1,clip+1)=color;
          end
        elseif strcmp(m.path,'/live/arm')
        elseif strcmp(m.path,'/live/mute')
        elseif strcmp(m.path,'/live/solo')
        elseif strcmp(m.path,'/live/volume')
        elseif strcmp(m.path,'/live/pan')
        elseif strcmp(m.path,'/live/send')
        elseif strcmp(m.path,'/live/master/volume')
          if obj.debug
            fprintf('Got %s\n',formatmsg(m.path,m.data));
          end
          obj.volume=m.data{1};
          oscmsgout(obj.oscdests,'/volume',{obj.volume});
          oscmsgout(obj.oscdests,'/volume/value',{sprintf('%.1f',obj.slider2db(obj.volume))});
        elseif strcmp(m.path,'/live/master/pan')
        elseif strcmp(m.path,'/live/master/crossfader')
        elseif strcmp(m.path,'/live/return/mute')
        elseif strcmp(m.path,'/live/return/solo')
        elseif strcmp(m.path,'/live/return/volume')
        elseif strcmp(m.path,'/live/return/pan')
        elseif strcmp(m.path,'/live/return/send')
        elseif strcmp(m.path,'/live/overdub')
        elseif strcmp(m.path,'/live/tempo')
          obj.pll.settempo(m.data{1});
          oscmsgout('LD','/live/tempo',{int32(m.data{1})});
          oscmsgout(obj.oscdests,'/tempo',{int32(m.data{1})});
          oscmsgout(obj.oscdests,'/tempo/value',{int32(m.data{1})});
        elseif strcmp(m.path,'/live/scene')
        elseif strcmp(m.path,'/live/scenes')
          obj.numscenes=m.data{1};
          fprintf('Num scenes=%d\n', obj.numscenes);
        elseif strcmp(m.path,'/live/track')
        elseif strcmp(m.path,'/live/master/meter')
          %fprintf('Master meter(%d)=%.3f\n', m.data{1},m.data{2});
          oscmsgout(obj.oscdests,sprintf('/meter/%d',m.data{1}+1),{m.data{2}});
        elseif strcmp(m.path,'/live/return/meter')
        elseif strcmp(m.path,'/live/track/meter')
          %fprintf('Track%d meter(%d)=%.3f\n', m.data{1},m.data{2},m.data{3});
          [song,songtrack]=obj.getsong(m.data{1}+1);
          if isempty(song)
            % This often happens since there is a meter for a group which is not in the list
            %fprintf('Bad song: Track%d meter(%d)=%.3f\n', m.data{1},m.data{2},m.data{3});
          else
            db=obj.slider2db(m.data{3});
            oscmsgout('LD','/live/songtrack/meter',{song,songtrack,m.data{2}+1,m.data{3},db});
          end
        elseif strcmp(m.path,'/live/device/param')
        elseif strcmp(m.path,'/live/return/device/param')
        elseif strcmp(m.path,'/live/master/device/param')
        elseif strcmp(m.path,'/live/device/selected')
        elseif strcmp(m.path,'/live/return/device/selected')
        elseif strcmp(m.path,'/live/master/device/selected')
        elseif strcmp(m.path,'/live/beat')
          obj.pll.setref(m.data{1});
          oscmsgout({'LD'},'/live/beat',m.data);
        elseif strcmp(m.path,'/live/name/trackblock')
        elseif strcmp(m.path,'/remix/error')
        else
          fprintf('Unrecognized reply from AL: %s\n', formatmsg(m.path,m.data));
        end
      end
      if nmsg>0 && obj.debug>0
        fprintf('Received %d messages from AL\n', nmsg);
      end
    end

    function assignsongs(obj)
    % Additional info not visible from AL
    % Each song is {ID,name,tempo}
      songmap={{'NG','New Gamelan',120}
               {'DB','Deep Blue',100}
               {'FI','Firebell',108}
               {'FO','Forski',72}
               {'GA','Garage Revisited',120}
               {'MB','Music Box',111}
               {'EP','Episarch',93}
               {'OL','Oluminum',93}
               {'PR','Pring',108}
               {'QU','Quetzal',120}
               {'AN','Animals',120}
               {'MV','Movies',120}};
                    
      % Assign songs based on track names - song tracks are of the form %2c%d - ignore all others
      obj.songs={};
      obj.songids={};
      obj.songs=[];
      
      for i=1:length(obj.tracknames)
        nm=obj.tracknames{i};
        if length(nm)>=3 && nm(1)>='A' && nm(1)<='Z' && nm(2)>='A' && nm(2)<='Z' && ((nm(3)>='0' && nm(3)<='9') || (length(nm)>=4 && nm(3)==' ' && nm(4)>='0' && nm(4)<='9'))
          songid=nm(1:2);
          songtrack=str2double(nm(3:end));
          song=obj.findsongid(songid);
          if isempty(song)
            songmapentry=[];
            for j=1:length(songmap)
              if strcmp(songmap{j}{1},songid)
                songmapentry=j;
                break;
              end
            end
            if isempty(songmapentry)
              fprintf('No entry for name or tempo of song with ID %s; ignoring\n', songid);
              continue;
            end
            obj.songs{end+1}=[];   % New song;
            obj.songnames{end+1}=songmap{songmapentry}{2};
            obj.songtempo(end+1)=songmap{songmapentry}{3};
            obj.songids{end+1}=songid;
            song=length(obj.songs);
            fprintf('AL: Added song %d: %s with ID %s, tempo %d\n', song, obj.songnames{end},obj.songids{end},obj.songtempo(end));
          end
          
          % Set mapping
          obj.songs{song}(songtrack)=i;
        else
          if ~strcmp(nm,'Empty')
            fprintf('AL: Skipping track %d: %s that is not a song track\n', i, nm);
          end
        end
      end

      % Check that there are no missing songtracks
      for i=1:length(obj.songs)
        if any(obj.songs{i}==0)
          for k=find(obj.songs{i}==0)
            fprintf('AL: Warning: missing track in AL for %s%d\n', obj.songids{i}, k);
          end
          obj.songs{i}=obj.songs{i}(obj.songs{i}>0);   % Compact list
        end
      end
      
      fprintf('AL: Assigned %d songs\n', length(obj.songs));
    end
    
    function stop(obj)
      oscmsgout('AL','/live/stop',{});
      obj.clearui();
    end
    
    function stoptrack(obj,track)
      oscmsgout('AL','/live/stop/track',{int32(track-1)});
    end
    
    function stopsongtrack(obj,song,songtrack)
      obj.stoptrack(obj.gettrack(song,songtrack));
    end
    
    function stopalltracks(obj)
      for track=1:obj.numtracks()
        % Make sure all Ableton channels are off
        obj.stoptrack(track);
      end
    end

    function settempo(obj,tempo)
      oscmsgout('AL','/live/tempo',{int32(tempo)});
      obj.pll.settempo(tempo);
    end
    
    function setvolume(obj,volume)
      oscmsgout('AL','/live/master/volume',{volume});
    end
    
    function playclip(obj,track,scene)
      oscmsgout('AL','/live/play/clip',{int32(track-1),int32(scene-1)});
    end

    function playsongclip(obj,song,songtrack,scene)
      obj.playclip(obj.gettrack(song,songtrack),scene);
    end

    function cliptrigger(obj,song,track,clip)
    % Trigger a clip on given track in Ableton Live (via LiveOSC)
    % Track is track number starts with 1
    % Clip is one in the scene number in AL (starts with 1)
      nm=obj.getsongclipname(song,track,clip);
      if obj.debug
        fprintf('cliptrigger: song=%d,track=%d (%s),clip=%d (%s),tempo=%d\n', ...
                song,track, obj.getsongtrackname(song,track), clip, nm, obj.getsongtempo(song));
      end
      if isempty(nm)
        obj.stopsongtrack(song,track);
      else
        obj.playsongclip(song,track,clip);
      end
    end

    function clearui(obj)
      for channel=1:8
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/scene',channel),{''});
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/clip',channel),{''});
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/track',channel),{''});
        oscmsgout(obj.oscdests,sprintf('/grid/table/%d/pos',channel),{''});
      end
    end
    
    function beat=getbeat(obj)
      beat=obj.pll.getbeat;
    end

    function status=isplaying(obj,track)
      if track<1 || track>length(obj.trackstatus)
        fprintf('isplaying: Bad track %d, len(obj.trackstatus)=%d\n', track, length(obj.trackstatus));
        status=false;
        return;
      end
      if obj.trackstatus(track)~=1
        status=true;
      else
        status=false;
      end
    end
        
  end

  methods(Static)
    function db=slider2db(slider)
    % Convert 0.0-1.0 volume slider to dB value same as AL does
      map=[0.0142	-65
           0.0346	-60
           0.0913	-50
           0.1	-48.6
           0.1162	-46
           0.1429	-42
           0.171	-38
           0.2	-34.4
           0.2032	-34
           0.2384	-30
           0.2791	-26
           0.3	-24.2
           0.3288	-22
           0.4	-18.0
           0.5	-14
           0.6	-10
           0.7	-6
           0.8	-2
           0.9	2
           1	6];
      db=interp1(map(:,1),map(:,2),slider,'linear','extrap');
      if slider>1
        db=6;
      elseif slider<=0
        db=-inf;
      end
    end
  end
end