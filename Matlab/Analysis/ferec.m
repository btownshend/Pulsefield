classdef ferec < handle
  properties
    filename
    data
  end

  methods
    function obj=ferec(filename)
      obj.load(filename);
    end

    function load(obj,filename)
    % Load a .ferec file
      obj.filename=filename;
      obj.data=[];
      fd=fopen(filename,'r');
      if fd<0
        error('Unable to open %s',filename);
      end
      while true
        line=fgetl(fd);
        if line==-1
          break;
        end
        [a,cnt]=sscanf(line,'%d %d %d %d %d %d');
        assert(cnt==6);
        x=struct('unit',a(1),'frame',a(2),'time',a(3)+a(4)/1e6,'xx',a(5));
        nscan=a(6);
        line=fgetl(fd);
        assert(line(1)=='D');
        x.d=sscanf(line(2:end),'%d ');
        line=fgetl(fd);
        assert(line(1)=='R');
        x.r=sscanf(line(2:end),'%d ');
        obj.data=[obj.data,x];
      end
    end

    function plot(obj)
      setfig(obj.filename);clf;
      units=sort(unique([obj.data.unit]));
      leg={};
      fps=50;
      t0=min([obj.data.time]);
      for i=1:length(units)
        sel=[obj.data.unit]==units(i);
        d=obj.data(sel);
        if i==1
          fps=1/mean(diff([d.time])./diff([d.frame]));
        end
        frame=[d.frame]-d(1).frame;
        time=[d.time]-t0;
        offset=median(time-frame/fps);
        plot(frame,time-frame/fps-offset,'.');
        hold on;
        leg{end+1}=sprintf('Unit%d offset=%.2fmsec',units(i),offset*1000);
      end
      legend(leg);
      xlabel('Frame');
      ylabel('Timing error (sec)');
      title(sprintf('%s %.5f FPS',obj.filename, fps),'Interpreter','none');
      c=axis;
      for y=ceil(c(3)*fps):floor(c(4)*fps)
        plot(c(1:2),y/fps*[1,1],':');
      end
    end
  end
end
