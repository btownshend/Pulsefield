% Initialize current state

function info=infoinit()
  % Setup info structure to pass things into apps
  info=struct('duration',120,'velocity',127,'multichannel',1,'volume',1,'tempo',120,'scale',1,'key',1,'pgm',ones(1,6),'refresh',true,'cm',ChannelMap(8),'preset',0,'starttime',now,'ableton',1,'max',1,'al',Ableton());
  % Ableton, Max flags indicate whether notes/sounds should be played -- control messages still sent
  % Channel(k) is the id that is on channel k, or 0 if not in use
  [~,us]=getscales();
  info.scales={us.name};
  info.keys={'C','C#','D','D#','E','F','F#','G','G#','A','A#','B'};
  info.pgms=gmpgmnames();

  % Table of apps
  info.apps=struct('name',{'Guitar',   'CircSeq','HotSpots',   'Grid'},...
                   'fn'  ,{@app_guitar,@app_cseq,@app_hotspots,@app_grid},...
                   'pos' ,{'5/1',      '5/2',     '5/3',       '5/4'});
  for i=1:length(info.apps)
    info.apps(i).index=i;
  end

  info.currapp=info.apps(end);

  % Table of sample sets
  info.samples=struct('name',{'MIDI',   'A','B',   'C'},...
                      'pos' ,{'5/1',      '5/2',     '5/3',       '5/4'});
  for i=1:length(info.samples)
    info.samples(i).index=i;
  end

  info.currsample=info.samples(1);

  % Table of presets (set of program numbers for each channel)
  % Can be shorter than pgms[], only sets first N
  info.presets={[1 1 1 1 1 1],25:2:35,[53,54,55,86,92],41:46};
  info.presetnames={'Piano','Guitar','Voices','Strings'};

  % Flags
  info.needcal = false;
  info.quit = false;

  info.juststarted = true;
end
