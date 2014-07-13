% Background model
classdef Background < handle
    properties
      angle;
      range;
      freq;
      sigma;
    end

    methods
      function bg=Background(vis)
        if nargin<1
          return;	% For empty constructor
        end
        params=getparams();
        bg.angle=vis.angle;
        for i=1:3
          bg.range(i,:)=zeros(1,length(vis.range));
          bg.freq(i,:)=zeros(1,length(vis.range));
        end
      end
        
      function obj=loadobj(obj)
        if isstruct(obj)
          newobj=Background;
          newobj.angle=obj.angle;
          newobj.range=obj.range;
          newobj.freq=obj.freq;
          newobj.sigma=obj.sigma;
          obj=newobj;
        end
      end

      function bg=clone(obj)
        bg=Background();
        bg.angle=obj.angle;
        bg.range=obj.range;
        bg.freq=obj.freq;
      end
        
      function is=isbg(bg,vis)
        params=getparams();
        r=vis.range(:,1,:);r=r(:)';
        % One of the 2 most frequent ranges
        is1=abs(r-bg.range(1,:))<params.minbgsep;
        % Or near adjacent scan bg
        is1=is1|abs(r-bg.range(1,[2:end,end]))<params.minbgsep;
        is1=is1|abs(r-bg.range(1,[1,1:end-1]))<params.minbgsep;
        % Same for 2nd most freq
        is2=abs(r-bg.range(2,:))<params.minbgsep;
        % AND, between adjacent most frequent ranges
        % This way, the 2nd most frequent range is only used if there is a plausible reason for it (laser picking up near bg in some frames, far bg in others)
        is2=is2 & ((r>bg.range(1,[1,1:end-1]) ~= (r>bg.range(1,[2:end,end]))));
        is=is1|is2|r>=params.maxrange|r<params.minrange;
      end
      
      function is=isoutside(bg,vis)
        r=vis.range(:,1,:);r=r(:)';
        is=r>max(bg.range(1:2,:),1);
      end
      
      function update(bg,vis,varargin)
        defaults=struct('debug',false);
        args=processargs(defaults,varargin);
        params=getparams();
        r=vis.range(:,1,:); r=r(:)';
        
        isoutside=r>=params.maxrange| r<params.minrange;
        isbg(1,:)=~isoutside & (abs(bg.range(1,:)-r) < params.minbgsep);
        isbg(2,:)=~isoutside & ~isbg(1,:) & abs(bg.range(2,:)-r) < params.minbgsep;
        isbg(3,:)=~isoutside & ~isbg(1,:) & ~isbg(2,:) & abs(bg.range(3,:)-r) < params.minbgsep;

        % Averaging for closest matched bg
        for i=1:size(bg.range,1)
          bg.range(i,isbg(i,:))=r(isbg(i,:))/params.tc+bg.range(i,isbg(i,:))*(1-1/params.tc);
          bg.freq(i,:)=(bg.freq(i,:)*(params.tc-1)+isbg(i,:))/params.tc;
        end

        % Reset bg3 if there are no matches
        newpt=not(any(isbg,1))&~isoutside;
        bg.freq(end,newpt)=1/params.tc;   % Reset count if we're not close to new background
        bg.range(end,newpt)=r(newpt);   % Reset to current position for points far from any of the three

        % Swap things when bg(n) frequency is higher than bg(n-1)
        for i=2:-1:1
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
      params=getparams();
      if nargin<3
        minfreq=params.minbgfreq;
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
          plot(bg.angle([1,end]),params.minbgfreq*[1,1],':');
        end
        title('Frequency');
        suptitle(sprintf('After snap %d',s));
        input('Press return to continue','s');
      end
    end

    function p=getActiveArea(bg,maxrange)
    % Estimate the active area -- a polynomial enclosed by the "walls"
      filtlen=round(size(bg.range,2)/12);
      mr=medfilt1(bg.range(1,:),filtlen);
      sel=mr<=maxrange;
      p=range2xy(bg.angle(sel),mr(sel));
      p(end+1,:)=p(1,:);
    end

   end
end
