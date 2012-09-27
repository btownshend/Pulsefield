% Abstract base class for sound generation apps
classdef (Abstract) SoundApp < handle
  properties
    name;
    uiposition;
    debug=true;
  end
  methods
    function obj=SoundApp(uiposition)
      obj.uiposition=uiposition;
    end
    
    function nm=getname(obj)
      nm=obj.name;
    end
  
    function plot(obj,p,info)
    % Default to no-op
    end
  end
  
  methods (Abstract)
    info=start(obj,p);
    stop(obj,p);
    info=update(obj,p,info);
  end
end
