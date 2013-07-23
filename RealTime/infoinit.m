% Initialize current state

function info=infoinit()
  % Setup info structure to pass things into apps
  info=struct('duration',120,'velocity',127,'volume',0.5,'tempo',120,'scale',1,'key',1,'pgm',ones(1,8),'refresh',true,'cm',ChannelMap(8),'preset',0,'starttime',now,'ableton',0,'max',0,'al',Ableton('ableton.mat'),'song',[],'touchpage','','health',Health({'FE','LD','AL','MX','TO','VD'}),'lastping',0,'pinginterval',1.0,'latency',0.0,'groupmap',GroupMap(60));

  % Ableton, Max flags indicate whether notes/sounds should be played -- control messages still sent
  % Channel(k) is the id that is on channel k, or 0 if not in use
  [~,us]=getscales();
  info.scales={us.name};
  info.keys={'C','C#','D','D#','E','F','F#','G','G#','A','A#','B'};
  info.pgms=gmpgmnames();

  % Set current song
  if info.al.numsongs() > 0
    info.song=1;
  end
  
  % Table of apps
  info.apps={ SoundAppGuitar('5/1'), SoundAppCSeq('5/2'), SoundAppHotspots('5/3'), SoundAppGrid('5/4') };
  info.currapp=info.apps{end};

  % Table of sample sets
  info.samples=struct('name',{'MIDI',   'A','B',   'C'},...
                      'pos' ,{'5/1',      '5/2',     '5/3',       '5/4'});
  for i=1:length(info.samples)
    info.samples(i).index=i;
  end
  info.currsample=info.samples(1);

  % Table of presets (set of program numbers for each channel)
  % Can be shorter than pgms[], only sets first N
  info.presets={[1 1 1 1 1 1 1 1],25:2:39,[53,54,55,86,92,92,92],41:48};
  info.presetnames={'Piano','Guitar','Voices','Strings'};

  % Flags
  info.needcal = 0;
  info.quit = false;

  info.juststarted = true;
end
