% Background model
classdef Background < handle
    properties
      angle;
      range;
      freq;
      minsep;
      tc;
      minbgfreq;
    end

    methods
      function bg=Background(vis,varargin)
        defaults=struct('minsep',0.1,'tc',50*60,'minbgfreq',0.01,'debug',false);
        args=processargs(defaults,varargin);
        bg.angle=vis.angle;
        r=vis.range(:,1,:); r=r(:);
        for i=1:3
          bg.range(i,:)=r;
          bg.freq(i,:)=0*r;
        end
        bg.freq(1,:)=1;
        bg.minsep=args.minsep;
        bg.tc=args.tc;
        bg.minbgfreq=args.minbgfreq;
      end
        
      function is=isbg(bg,vis)
        r=vis.range(:,1,:);r=r(:)';
        is=abs(r-bg.range(1,:))<bg.minsep | (bg.freq(2,:) >bg.minbgfreq & abs(r-bg.range(2,:))<bg.minsep);
      end
      
      function is=isoutside(bg,vis)
        r=vis.range(:,1,:);r=r(:)';
        is=r>max(bg.range(1:2,:),1);
      end
      
      function update(bg,vis,varargin)
        defaults=struct('debug',false);
        args=processargs(defaults,varargin);
        r=vis.range(:,1,:); r=r(:)';

        isbg(1,:)=abs(bg.range(1,:)-r) < bg.minsep;
        isbg(2,:)=~isbg(1,:) & abs(bg.range(2,:)-r) < bg.minsep;
        isbg(3,:)=~isbg(1,:) & ~isbg(2,:) & abs(bg.range(3,:)-r) < bg.minsep;

        % Averaging for closest matched bg
        for i=1:size(bg.range,1)
          bg.range(i,isbg(i,:))=r(isbg(i,:))/bg.tc+bg.range(i,isbg(i,:))*(1-1/bg.tc);
          bg.freq(i,:)=(bg.freq(i,:)*(bg.tc-1)+isbg(i,:))/bg.tc;
        end

        % Reset bg3 if there are no matches
        newpt=not(any(isbg,1));
        bg.freq(end,newpt)=1/bg.tc;   % Reset count if we're not close to new background
        bg.range(end,newpt)=r(newpt);   % Reset to current position for points far from any of the three

        % Swap things when bg(n) frequency is higher than bg(n-1)
        for i=1:2
          swap=bg.freq(i+1,:)>bg.freq(i,:);
          if sum(swap)>0
            if args.debug
              fprintf('Swapping %d background pixels between rank %d and %d\n', sum(swap), i, i+1);
            end
            [bg.freq(i,swap),bg.freq(i+1,swap)]=deal(bg.freq(i+1,swap),bg.freq(i,swap));
            [bg.range(i,swap),bg.range(i+1,swap)]=deal(bg.range(i+1,swap),bg.range(i,swap));
        end
      end
    end
    
    function test(bg,snap,minfreq)
      if nargin<3
        minfreq=bg.minbgfreq;
      end
      col='rgb';
      for s=1:100:length(snap)-99
        for k=0:99
          bg.update(snap(s+k).vis);
        end
        setfig('bg.test');clf;
        for i=1:3
          subplot(3,3,i);
          sel=bg.freq(i,:)>minfreq;
          xy=range2xy(bg.angle(sel),bg.range(i,sel));
          plot(xy(:,1),xy(:,2),['.',col(i)]);
          hold on;
          title(sprintf('bg%d - N>%.3f = %d',i,minfreq,sum(sel)));
          axis equal
          if i==1
            c=axis;
          else
            axis(c);
          end
        end
        subplot(3,3,[4,5,6]);
        for i=1:3
          plot(bg.angle,bg.range(i,:),['.',col(i)]);
          hold on;
        end
        title('Range');
        legend('1','2','3');
        subplot(3,3,[7,8,9]);
        for i=1:3
          semilogy(bg.angle,bg.freq(i,:),col(i));
          hold on;
          plot(bg.angle([1,end]),bg.minbgfreq*[1,1],':');
        end
        title('Frequency');
        suptitle(sprintf('After snap %d',s));
        input('Press return to continue','s');
      end
    end
   end
end
