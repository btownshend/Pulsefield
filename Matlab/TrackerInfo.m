classdef TrackerInfo < handle
  properties
    when;
    elapsed;  % Time in seconds from start
    frame;
    app;
    apps;
    appindex;
    npeople;
    draw;
    sched;
    total;
    free;
    feframerate;
    framerate;
    load;
    tp;
  end
  methods
    function obj=TrackerInfo(file)
      [d,obj.frame,obj.app,obj.npeople,obj.draw,obj.sched,obj.total,obj.free,tp0,tp1,tp2,tp3,tp4,tp5,tp6,tp7]=textread(file,'%s %d %s %d %d %d %f %f %d %d %d %d %d %d %d %d',-1,'headerlines',2,'delimiter','\t');
      try
        obj.when=datenum(d(1:end-1),'yyyy/mm/dd HH:MM:SS');
        obj.when(end+1)=datenum(d{end},'yyyy/mm/dd HH:MM:SS');
      catch me
        ;
      end
      obj.elapsed=(obj.when-obj.when(1))*24*60*60;  % Elapsed in seconds
      obj.feframerate=(obj.frame(end)-obj.frame(1))/obj.elapsed(end);
      obj.elapsed=obj.frame/obj.feframerate;   % Higher precision
      obj.apps=unique(obj.app,'stable');
      obj.appindex=nan(size(obj.app));
      for i=1:length(obj.apps)
        obj.appindex(strcmp(obj.app,obj.apps{i}))=i;
      end
      obj.load=obj.draw./(obj.draw+obj.sched);
      obj.framerate=1000./(obj.draw+obj.sched);
      obj.tp=tp0;
      obj.tp(1:length(tp1),end+1)=tp1;
      obj.tp(1:length(tp2),end+1)=tp2;
      obj.tp(1:length(tp3),end+1)=tp3;
      obj.tp(1:length(tp4),end+1)=tp4;
      obj.tp(1:length(tp5),end+1)=tp5;
      obj.tp(1:length(tp6),end+1)=tp6;
      obj.tp(1:length(tp7),end+1)=tp7;
    end
    
    function plotstats(obj)
    % Plot draw/sched time
      setfig('time');clf;
      ax=subplot(311);
      plot(obj.elapsed,(obj.draw+obj.sched)/1e6);
      xlabel('Elapsed (sec)');
      ylabel('Time (msec)');
      title('Draw+Sched Time');
      hold on;
      yyaxis right
      plot(obj.elapsed,obj.appindex);
      set(gca,'YTick',1:length(obj.apps));
      set(gca,'YTickLabels',obj.apps);

      ax(2)=subplot(312);
      medlen=50;
      for i=1:size(obj.tp,2)
          plot(obj.elapsed,obj.tp(:,i)./obj.draw*100);
          hold on;
      end
      plot(obj.elapsed,100*movmedian(obj.load,medlen));
      hold on;
      xlabel('Elapsed (sec)');
      ylabel('Load %');
      yyaxis right
      plot(obj.elapsed,obj.npeople);
      ylabel('Num People');
      title('Load ');
      
      ax(3)=subplot(313);
      plot(obj.elapsed,obj.free);
      hold on;
      plot(obj.elapsed,obj.total);
      ylabel('MBytes');
      xlabel('Elapsed (sec)');
      legend('free','total');
      
      linkaxes(ax,'x');
    end
    
    function plotloads(obj,appsel)
      setfig('loads');clf;
      legs={};
      if nargin<2
        appsel=ones(size(obj.apps));
      end
      for i=1:length(obj.apps)
        if ~appsel(i)
          continue;
        end
        sel=obj.appindex==i;
        if sum(sel)<1000
          continue;
        end
        cdfplot(movmedian(obj.framerate(sel),50));
        hold on;
        legs{end+1}=sprintf('%s %.0f%% %.0fFPS',obj.apps{i},100*median(obj.load(sel)),median(obj.framerate(sel)));
      end
      legend(legs,'location','best');
    end
    
    function stalls(obj,mintime)
    % Show info about stalls
      if nargin<2
        mintime=1.0;  % 1 sec
      end
      sel=find(obj.draw+obj.sched > 1e9*mintime);
      fprintf('Found %d stalls of >= %.1f seconds:  %s\n', length(sel), mintime, sprintf('%.1f ', (obj.draw(sel)+obj.sched(sel))/1e9));
      sel=sort(unique([sel;sel-1]));
      for ii=1: length(sel)
        i=sel(ii);
        fprintf('%d %s frame %d draw=%.2f, sched=%.2f, tp=(%s)\n', i, datestr(obj.when(i)), obj.frame(i), obj.draw(i)/1e9, obj.sched(i)/1e9, sprintf('%.1f ',100*obj.tp(i,:)/obj.draw(i)));
      end
    end
  end
end