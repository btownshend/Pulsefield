% Class for a digital PLL to lock to OSC beat clocks
classdef PLL < handle
  properties
    bpm=120;   % Nominal speed
    freq=2.0;  % Exact tracking beat/sec
    clkbeat=0;
    clktime=0;
    tc=0.01;
    fgain=.5;
    pdlpf=0;
  end
  
  methods
    
    function obj=PLL
    end
    
    function settempo(obj,bpm)
    % Set nominal speed
      obj.bpm=bpm;
    end
    
    function setref(obj,beat,when)
    % Set reference to beat at time when
    % Use 'now' if one arg
      if nargin<3
        when=now;
      end
      mybeat=obj.getbeat(when);
      % Phase difference
      pd=beat-mybeat;
      if obj.clktime==0 || abs(pd)>1
        if obj.clktime~=0
          fprintf('Large phase difference (%.2f beats) - jumping\n', abs(pd));
        end
        obj.clkbeat=beat;
        obj.clktime=when;
        obj.pdlpf=0;
      else
        % Low-pass filtered phase difference
        obj.pdlpf=(1-obj.tc)*obj.pdlpf+obj.tc*pd;
      end

      % VCO
      obj.freq=obj.bpm/60+obj.fgain*obj.pdlpf;
    end

    function beat=getbeat(obj,when)
    % Get current beat value
      if nargin<2
        when=now;
      end
      obj.clkbeat=obj.clkbeat+(when-obj.clktime)*24*3600*obj.freq;
      obj.clktime=when;
      beat=obj.clkbeat;
    end
  end
  
end
