classdef World < handle
  properties
    tracks;
    nextid;
    npeople;
    assignments;	% vector of length of scan which indicates which ID and leg this scan is attributed to, 0 for background
    like;		% NSCAN x NID x NLEGS: assignment likelihoods
    bglike;		% Background likelihood
    bestlike;		% NSCAN: best likelihood after assignment
    debug;
  end
  
  methods
    function obj=World()
      obj.tracks=[];
      obj.nextid=1;
      obj.debug=false;
    end

    function obj=loadobj(obj)
      if isstruct(obj)
        newobj=World;
        fn=fieldnames(obj);
        for i=1:length(fn)
          newobj.(fn{i})=obj.(fn{i});
        end
        if ~isfield(obj,'debug')
          newobj.debug=false;
        end
        obj=newobj;
      end
    end

    function w=clone(obj)
      w=World();
      for i=1:length(obj.tracks)
        w.tracks=[w.tracks,obj.tracks(i).clone()];
      end
      w.nextid=obj.nextid;
    end
      
    function predict(obj,nsteps,fps)
      for i=1:length(obj.tracks)
        obj.tracks(i).predict(nsteps,fps);
      end
    end
    
    function makeassignments(obj,vis) 
    % Calculate likelihoods of each scan belonging to each track
      params=getparams();
      params.legdiamstd=0.05;
      xy=range2xy(vis.angle,vis.range);
      obj.like=nan(size(xy,1),length(obj.tracks),2);
      obj.bglike=-log(vis.bgprob);
      for f=1:size(xy,1)
        for it=1:length(obj.tracks)
          t=obj.tracks(it);
          for i=1:2
            dpt=sqrt((t.legs(i,1)-xy(f,1)).^2+(t.legs(i,2)-xy(f,2)).^2);
            % This is a fudge since the DIAMETER is log-normal and the position itself is normal
            obj.like(f,it,i)=normlike([t.legdiam/2,params.legdiamstd/2+t.posvar(i)],dpt);
            % Check if the intersection point would be shadowed by the object (ie the contact is on the wrong side)
            dclr=segment2pt([0,0],xy(f,:),t.legs(i,:));
            if dclr<dpt && obj.like(f,it,i)<5
              clike=-log(normcdf(dclr,t.legdiam/2,params.legdiamstd/2+t.posvar(i)));
              obj.like(f,it,i)=obj.like(f,it,i)+clike;
            end
          end
        end
        ptf=squeeze(obj.like(f,:,:));
        ptf(2:end+1,:)=ptf;  % Shift to put background on front
        ptf(1,1)=obj.bglike(f);
        ptf(1,2)=inf;
        [obj.bestlike(f),b]=min(ptf(:));
        [ass,obj.assignments(f,2)]=ind2sub(size(ptf),b);
        obj.assignments(f,1)=ass-1;  
      end
    end

    function plotassignments(obj) 
      setfig('classlike');clf;
      for it=1:length(obj.tracks)
        subplot(length(obj.tracks),1,it);
        plot(obj.bglike,'k');
        hold on;
        plot(obj.like(:,it,1),'r');
        plot(obj.like(:,it,2),'g');
        title(sprintf('ID %d',obj.tracks(it).id));
        c=axis;
        axis([c(1),c(2),-1,30]);
      end
      setfig('plotassignments');clf;
      subplot(311);
      ids=[obj.tracks.id];
      ids=[0,ids];	% 0=background
      plot(ids(obj.assignments(:,1)+1));
      title('ID of best like');
      subplot(312);
      plot(obj.assignments(:,2));
      title('Leg of best like');
      subplot(313);
      plot(obj.bestlike);
      title('Best likelihood');
    end
    
    function update(obj,vis,nsteps,fps)
      params=getparams();
      obj.predict(nsteps,fps);
      
      MAXSPECIAL=2;
      maxclass=max([1;vis.class]);
      if obj.debug&&maxclass>MAXSPECIAL
        fprintf('Assigning classes:  ');
        for i=MAXSPECIAL+1:maxclass
          meanpos=mean(vis.xy(vis.class==i,:),1);
          fprintf('%d@(%.2f,%.2f) ',i,meanpos);
          if any(isnan(meanpos))
            keyboard;
          end
        end
        fprintf('\n');
      end
      like=nan(length(obj.tracks),maxclass,maxclass);
      for p=1:length(obj.tracks)
        like(p,:,:)=obj.tracks(p).getclasslike(vis);
        if obj.debug
          fprintf('Person %2d like: %s\n', obj.tracks(p).id);
          for i=[1,3:size(like,2)]
            fprintf('C%d:[%.1f / %s] ',i,like(p,i,1),sprintf('%.1f ', like(p,i,3:end)));
          end
          fprintf('\n');
        end
      end

      % Greedy assignment
      assign=nan(maxclass,2);
      for k=1:length(obj.tracks)
        [minlike,maxind]=min(like(:));
        [p,i,j]=ind2sub(size(like),maxind);
        assign(p,:)=[i,j];
        if obj.debug
          fprintf('Assigned classes %d,%d to person %d with loglike=%f\n', i,j,obj.tracks(p).id,-minlike);
        end
        obj.tracks(p).update(vis,i,j,nsteps,fps);
        like(p,:,:)=inf;
        if i>1
          like(:,i,:)=inf;
          like(:,:,i)=inf;
        end
        if j>1
          like(:,j,:)=inf;
          like(:,:,j)=inf;
        end
      end
      % Deal with leftover classes
      otherclasses=setdiff([vis.class],assign(:));
      otherclasses=otherclasses(otherclasses>MAXSPECIAL);
      if ~isempty(otherclasses)
        for i=1:length(otherclasses)
          center(i,:)=mean(vis.xy(vis.class==otherclasses(i),:),1);
          if obj.debug
            fprintf('Class %d at (%.2f,%.2f) not assigned.\n', otherclasses(i), center(i,:));
          end
        end
        for i=1:length(otherclasses)
          if isnan(otherclasses(i))
            % Already consumed
            continue;
          end
          sel=vis.class==otherclasses(i);
          dist=sqrt((center(:,1)-center(i,1)).^2+(center(:,2)-center(i,2)).^2);
          dist(i)=inf;
          dist(isnan(otherclasses))=inf;
          [closest,j] = min(dist);
          if ~isempty(closest) && closest<params.newPersonPairMaxDist
            obj.tracks=[obj.tracks,Person(obj.nextid,vis,otherclasses(i),otherclasses(j),false)];
            obj.nextid=obj.nextid+1;
            otherclasses(j)=nan;
          elseif all(vis.xy(sel,2))>0 && all(vis.range(sel)<params.maxrange)
            obj.tracks=[obj.tracks,Person(obj.nextid,vis,otherclasses(i),[],false)];
            obj.nextid=obj.nextid+1;
          else
            fprintf('Ignoring class %d since it is solitary and partially out of range\n',otherclasses(i));
          end
        end
      end
  
      obj.deleteLostPeople();
    end
  
    function deleteLostPeople(obj)
      if isempty(obj.tracks)
        return;
      end
      params=getparams();
      
      % compute the fraction of the track's age for which it was visible
      ages = [obj.tracks(:).age];
      totalVisibleCounts = [obj.tracks(:).totalVisibleCount];
      visibility = totalVisibleCounts ./ ages;
      
      % find the indices of 'lost' people
      lostInds = (ages < params.ageThreshold & visibility < params.minVisibility) | ...
          [obj.tracks(:).consecutiveInvisibleCount] >= params.invisibleForTooLong;
      outsideInds = arrayfun(@(z) ~isempty(z.position) && (norm(z.position) > params.maxrange || z.position(2)<0),obj.tracks);
      if sum(outsideInds)>0
        fprintf('Deleting %d people out of range\n', sum(outsideInds));
      end
      fl=find(lostInds|outsideInds);
      for i=1:length(fl)
        t=obj.tracks(fl(i));
        fprintf('Deleting track %d at (%.2f,%.2f) with age %d, total %d, consec invis %d\n', ...
                t.id,t.position,t.age, t.totalVisibleCount,t.consecutiveInvisibleCount);
      end

      % delete lost people
      obj.tracks = obj.tracks(~(lostInds|outsideInds));
    end

  end % methods

end % class