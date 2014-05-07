classdef Person < handle
  properties
    id; 	% ID of person
    position; 	% Position of core (hips)
    legs;   	% Coordinates of legs;  legs(1,:)=left, legs(2,:)=right
    legsmeas;	% Measurement of legs (before any smoothing)
    prevlegs;	% Previous leg coordinates
    legvelocity;% Velocity of each leg
    scanpts;	% Scan points used to build each leg scanpts{1} for the left, scanpts{2} for the right
    legclasses;	% Classes assigned to legs (was class numbers, is now number of scan points)
    posvar;	% Estimated variance of position of legs 
    prevposvar;	% Previous frame posvar - useful for calculating variance of instantaneous velocity
    velocity;   % Overall velocity
                %facing;  	% Vector pointing forward
    legdiam;	% Estimate of leg diameter in meters
    leftness;	% Fraction of time leg(1) is on the left side of direction of motion
    maxlike;	% Maximum likelihood
    like;	% Cell array of likelihood matrices
    minval;	% Range of coordinates for likelihoods
    maxval;	
    age;
    consecutiveInvisibleCount;
    totalVisibleCount;
    debug;
  end
  
  methods
    function obj=Person(id,legs,debug);
    % Create a person with legs at the given positions
      if nargin<1
        return;   % For cloning
      end
      params=getparams();
      
      if nargin<3
        obj.debug=false;
      else
        obj.debug=debug;
      end
      
      if obj.debug
        fprintf('Creating person %d at (%.2f,%.2f),(%.2f,%.2f)\n',id, legs);
      end
      obj.id=id;
      obj.legdiam=params.initlegdiam;
      % Place the person centered on the point
      obj.legs=legs;
      obj.prevlegs=obj.legs;
      obj.legclasses=[1,1];
      obj.scanpts={};
      obj.position=mean(obj.legs,1);
      obj.posvar=params.initialPositionVar*[1,1];
      obj.prevposvar=obj.posvar;
      obj.velocity=[0,0];
      obj.legvelocity=zeros(2,2);
      obj.leftness=0.0;
      obj.age=1;
      obj.consecutiveInvisibleCount=0;
      obj.totalVisibleCount=1;
      fprintf('Created %s\n',obj.tostring());
    end
    
    function obj=loadobj(obj)
      assert(isstruct(obj));
      if isstruct(obj)
        fn=fieldnames(obj);
        for j=1:length(obj)
          newobj(j)=Person;
          for i=1:length(fn)
            newobj(j).(fn{i})=obj(j).(fn{i});
          end
        end
        obj=newobj;
      end
    end

    function s=tostring(obj)
      s=sprintf('P%d at (%.2f,%.2f) with legs at (%.2f,%.2f)[%d], (%.2f,%.2f)[%d]; vel=(%.2f,%.2f),(%.2f,%.2f) posstd=(%.2f,%.2f), age=%d,cic=%d,tvc=%d', obj.id, obj.position,obj.legs(1,:), length(obj.scanpts{1}), obj.legs(2,:),length(obj.scanpts{2}), obj.legvelocity(1,:), obj.legvelocity(2,:),sqrt(obj.posvar),obj.age,obj.consecutiveInvisibleCount,obj.totalVisibleCount);
    end

    function p=clone(obj)
      p=Person();
      fn=fieldnames(p);
      for i=1:length(fn)
        p.(fn{i})=obj.(fn{i});
      end
    end
      
    function predict(obj,nstep,fps)
      params=getparams();
      if nstep>0
        % Step of zero is just a refine operation
        obj.prevlegs=obj.legs;
        obj.prevposvar=obj.posvar;
        obj.posvar=obj.posvar+params.driftVar*nstep;	% Amount of drift per unit step
        for i=1:2
          obj.legs(i,:)=obj.legs(i,:)+obj.legvelocity(i,:)*nstep/fps;
        end
        obj.position=mean(obj.legs,1);
      end
    end
    
    % OBSOLETE
    function [like,p1,p2]=getclasslike(obj,vis)
    % Compute like(i,j) which is likelihood that class i is leg1, class j is leg 2
    % like(i,1) and like(i,j) represents leg not being visible (shadowed)
      if obj.debug
        fprintf('getclasslike for ID %d\n', obj.id);
      end
      params=getparams();
      MAXSPECIAL=2;
      classes=unique(vis.class);
      classes=[1;classes(classes>MAXSPECIAL)];
      like=inf(max(classes),max(classes));
      for ii=1:length(classes)
        i=classes(ii);
        for jj=1:length(classes)
          j=classes(jj);
          if j==i && (j>1 || i>1)
            % Can't be same class
            % TODO: this could be the case!
            like(i,j)=inf;
            continue;
          end
          % Estimate the visible legs first
          if i==1
            p1=obj.legs(1,:);
          else
            p1=obj.circmodel(vis,i,false);
          end
          if j==1
            p2=obj.legs(2,:);
          else
            p2=obj.circmodel(vis,j,false);
          end
          % Check leg separation (add legdiam in case the estimates are off due to legs being partially shadowed)
          if ((i~=1 || j~=1) && norm(p2-p1)>params.maxlegsep+params.maxmovement) || (i~=1 && j~=1 && norm(p2-p1)>params.maxlegsep+obj.legdiam*2)
            % Too far apart!
            if obj.debug
              fprintf('Person %d, leg classes=(%d,%d) would give excessive leg sep of %.2f\n', obj.id, i, j, norm(p2-p1));
            end
            like(i,j)=inf;
            continue;
          end
          % Now do the hidden one
          if i==1
            p1=obj.nearestshadowed(vis,p2,params.maxlegsep,p1);
          end
          if j==1
            p2=obj.nearestshadowed(vis,p1,params.maxlegsep,p2);
          end
          l1=normlike([0,sqrt(obj.posvar(1))],norm(p1-obj.legs(1,:)));
          l2=normlike([0,sqrt(obj.posvar(2))],norm(p2-obj.legs(2,:)));
          if i==1
            l1=l1+params.hiddenPenalty;
          end
          if j==1
            l2=l2+params.hiddenPenalty;
          end
          like(i,j)=l1+l2;
        end
      end
    end
    
    function update(obj, vis, fs, nstep,fps);
    % Discretized likelihood calculation to match scan to track
    % Update positions of object using vis with scan lines as given in fs
      params=getparams();
      doplot=1;

      % Assume legdiam is log-normal (i.e. log(legdiam) ~ N(LOGDIAMMU,LOGDIAMSIGMA)
      LOGDIAMMU=log(obj.legdiam);
      LOGDIAMSIGMA=log(1+params.legdiamstd/obj.legdiam);

      xy=range2xy(vis.angle,vis.range);
      best=obj.legs;
      measvar=obj.posvar;
      step=0.02;

      if obj.debug
        fprintf('Initial leg positions (%.2f,%.2f) and (%.2f,%.2f)\n',best(1,:),best(2,:));
      end
      
      % Bound search by prior position + 2*sigma(position) + legdiam/2
      minval=min(obj.legs(1,:)-2*sqrt(obj.posvar(1)),obj.legs(2,:)-2*sqrt(obj.posvar(2)));
      maxval=max(obj.legs(1,:)+2*sqrt(obj.posvar(1)),obj.legs(2,:)+2*sqrt(obj.posvar(2)));
      fboth=[fs{1};fs{2}];
      if ~isempty(fboth)
        % Make sure any potential measured point is also in the search
        minval=min(minval,min(xy(fboth,:),[],1));
        maxval=max(maxval,max(xy(fboth,:),[],1));
      end
      minval=minval-obj.legdiam/2;
      maxval=maxval+obj.legdiam/2;
      
      minval=floor(minval/step)*step;
      maxval=ceil(maxval/step)*step;

      % Find the rays that will hit this box
      [theta(1),~]=xy2range([maxval(1),minval(2)]);
      [theta(2),~]=xy2range(minval);
      [theta(3),~]=xy2range(maxval);
      [theta(4),~]=xy2range([minval(1),maxval(2)]);
      clearsel=find(vis.angle>=min(theta) & vis.angle<=max(theta));
      if obj.debug
        fprintf('Clear path for angles %f:%f degrees; scans %d:%d\n',min(theta)*180/pi, max(theta)*180/pi, min(clearsel),max(clearsel));
      end


      nx=floor((maxval(1)-minval(1))/step+1.5);
      xvals=minval(1)+(0:nx-1)*step;
      ny=floor((maxval(2)-minval(2))/step+1.5);
      yvals=minval(2)+(0:ny-1)*step;
      if obj.debug
        fprintf('Range = (%.3f,%.3f): (%.3f,%.3f)\n', minval, maxval);
        fprintf('Searching over a %.0f x %.0f grid with %d,%d points/leg diam=%.2f +/- *%.2f\n', nx,ny,length(fs{1}),length(fs{2}),obj.legdiam, exp(LOGDIAMSIGMA));
      end

      adist=nan(nx,ny);
      dclr=nan(nx,ny);
      glike=nan(2,nx,ny);
      apriori=nan(2,nx,ny);
      clearlike=nan(2,nx,ny);

      for i=1:2
        for ix=1:nx
          x=xvals(ix);
          for iy=1:ny
            y=yvals(iy);
            adist(ix,iy)=norm(obj.legs(i,:)-[x,y]);
            dclr(ix,iy)=min(segment2pt([0,0],xy(clearsel,:),[x,y]));
            dpt=sqrt((xy(fs{i},1)-x).^2+(xy(fs{i},2)-y).^2);
            glike(i,ix,iy)=normlike([LOGDIAMMU,LOGDIAMSIGMA],log(dpt*2));
          end
        end
        apriori(i,:,:)=-log(normpdf(adist,0,sqrt(obj.posvar(i)+(params.sensorsigma^2))));
        clearlike(i,:,:)=-log(normcdf(log(dclr),LOGDIAMMU,LOGDIAMSIGMA));
      end

      glike=glike+clearlike;
      full=glike+apriori;

      for m=1:2
        bestlike=nan(1,2);
        for i=1:2
          [bestlike(i),bestind]=min(reshape(full(i,:,:),1,[]));
          [bestix,bestiy]=ind2sub([size(full,2),size(full,3)],bestind);
          best(i,:)=[minval(1)+step*(bestix-1),minval(2)+step*(bestiy-1)];
        end
        if obj.debug
          fprintf('Leg positions (%.2f,%.2f) with like=%f and (%.2f,%.2f) with like=%f, sep=%.2f\n',best(1,:), bestlike(1), best(2,:), bestlike(2),norm(best(1,:)-best(2,:)));
        end

        % Calculate measurement error
        measvar=zeros(1,2);
        tlike=zeros(1,2);
        for ix=1:nx
          x=xvals(ix);
          for iy=1:ny
            y=yvals(iy);
            for i=1:2
              dpt2=(best(i,1)-x).^2+(best(i,2)-y).^2;
              p=exp(-full(i,ix,iy));
              measvar(i)=measvar(i)+dpt2*p;
              tlike(i)=tlike(i)+p;
            end
          end
        end
        measvar=measvar./tlike;
        if obj.debug
          fprintf('Measurement std %.2f and %.2f\n',sqrt(measvar));
        end

        % Calculate separation likelihood using other leg at calculated position, variance
        for i=1:2
          legsepdist(i)=getlegsepdist(params.meanlegsep,params.legsepstd,sqrt(measvar(i)));
        end
        if obj.debug
          fprintf('Computing separation likelikehood using sep=%.2f\n', params.meanlegsep);
        end
        seplike=nan(2,nx,ny);
        d=nan(nx,ny);
        for i=1:2
          for ix=1:nx
            x=xvals(ix);
            for iy=1:ny
              y=yvals(iy);
              d(ix,iy)=sqrt((best(i,1)-x).^2+(best(i,2)-y).^2);
            end
          end
          seplike(3-i,:,:)=-log(interp1(legsepdist(i).d,legsepdist(i).p,d,'linear',1e-10));
        end
        % Repeat above calculations after updating full to include seplike
        full=glike+apriori+seplike;
      end
      
      if doplot
        setfig(sprintf('Calc discretelike ID %d',obj.id));clf;
        [mx,my]=meshgrid(minval(2):step:maxval(2),minval(1):step:maxval(1));
        sym={'<','>'};
        for i=1:8
          lnum=floor((i-1)/4)+1;
          if mod(i-1,4)==0
            like=apriori(lnum,:,:);
            ti='Apriori';
          elseif mod(i-1,4)==1
            like=glike(lnum,:,:);
            ti='Measurement';
          elseif mod(i-1,4)==2
            like=seplike(lnum,:,:);
            ti='Sep';
          else
            like=full(lnum,:,:);
            ti='Composite';
          end
          subplot(2,4,i);
          pcolor(my,mx,squeeze(like));
          shading('interp');
          hold on;
          plot(xy(fs{lnum},1),xy(fs{lnum},2),['w',sym{lnum}]);
          plot(best(lnum,1),best(lnum,2),'wx');
          plot(obj.legs(lnum,1),obj.legs(lnum,2),'wo');
          title(sprintf('%s %d',ti,lnum));
          if mod(i-1,4)==1 || mod(i-1,4)==3
            caxis(min(like(:))+[0,30]);
          end
          axis equal
        end
        suptitle(sprintf('ID %d, Frame %d',obj.id,vis.frame));
      end 

      % Apply the new values
      obj.like={squeeze(full(1,:,:)),squeeze(full(2,:,:))};
      obj.minval=minval;
      obj.maxval=maxval;
      obj.legs=best;
      obj.posvar=measvar;
      obj.legclasses=[length(fs{1}),length(fs{2})];
      obj.position=mean(obj.legs,1);
      obj.scanpts=fs;
      
      if nstep>0
        % nstep=0 is used to refine estimates

        % Update velocity
        if isempty(fboth)
          % Both legs hidden, maintain both at average velocity
          avgvelocity=mean(obj.legvelocity,1)*params.veldamping;
          obj.legvelocity(1,:)=avgvelocity;
          obj.legvelocity(2,:)=avgvelocity;
        else
          newlegvelocity=(obj.legs-obj.prevlegs)/(nstep/fps);
          if isempty(fs{1}) || obj.consecutiveInvisibleCount>0
            obj.legvelocity(1,:)=obj.legvelocity(1,:)*params.veldamping;
          else
            obj.legvelocity(1,:)=obj.legvelocity(1,:)*(1-1/params.velupdatetc)+newlegvelocity(1,:)/params.velupdatetc;
          end
          if isempty(fs{2}) || obj.consecutiveInvisibleCount>0
            obj.legvelocity(2,:)=obj.legvelocity(2,:)*params.veldamping;
          else
            obj.legvelocity(2,:)=obj.legvelocity(2,:)*(1-1/params.velupdatetc)+newlegvelocity(2,:)/params.velupdatetc;
          end
        end
        % Limit leg speed
        for k=1:2
          spd=norm(obj.legvelocity(k,:));
          if spd>params.maxlegspeed
            if obj.debug
              fprintf('P%d: Reducing leg%d speed from %.2f to %.2f\n', obj.id, k, spd, params.maxlegspeed);
            end
            obj.legvelocity(k,:)=obj.legvelocity(k,:)/spd*params.maxlegspeed;
          end
        end
        %fprintf('left =(%.2f,%.2f)->(%.2f,%.2f) nstep=%d, vel=(%.2f,%.2f)\n',obj.prevlegs(1,:),obj.legs(1,:),nstep,obj.legvelocity(1,:));
        %fprintf('right=(%.2f,%.2f)->(%.2f,%.2f) nstep=%d, vel=(%.2f,%.2f)\n',obj.prevlegs(2,:),obj.legs(2,:),nstep,obj.legvelocity(2,:));
        obj.velocity=mean(obj.legvelocity,1);

        % Compute 'leftness' to determine which leg is which
        prevpos=mean(obj.legs,1);
        delta=obj.position-prevpos;
        obj.leftness=obj.leftness*0.99+0.01*dot([-delta(2),delta(1)],diff(obj.legs,1));
        if false && obj.leftness<0.0
          % TODO: this causes the tracks to flip too often
          fprintf('Person %d: swapping legs: leftness=%.2f\n', obj.id, obj.leftness);
          obj.legs=obj.legs([2,1],:);
          obj.legvelocity=obj.legvelocity([2,1],:);
          obj.leftness=-obj.leftness;
        end

        % Increment age/visibilty counters
        if isempty(fs{1}) && isempty(fs{2})
          obj.consecutiveInvisibleCount=obj.consecutiveInvisibleCount+1;
        else
          obj.consecutiveInvisibleCount=0;
          obj.totalVisibleCount=obj.totalVisibleCount+1;
        end
        obj.age=obj.age+1;
      end
      if obj.debug
        fprintf('Updated %s\n', obj.tostring());
      end
    end

    function plotlike(obj,vis) 
      setfig(sprintf('discretelike ID %d',obj.id));clf;
      sym={'<','>'};
      for i=1:2
        subplot(1,2,i);
        xvals=((1:size(obj.like{i},2))-1)*(obj.maxval(i,1)-obj.minval(i,1))/(size(obj.like{i},2)-1)+obj.minval(i,1);
        yvals=((1:size(obj.like{i},1))-1)*(obj.maxval(i,2)-obj.minval(i,2))/(size(obj.like{i},1)-1)+obj.minval(i,2);
        [mx,my]=meshgrid(xvals,yvals);
        pcolor(mx,my,obj.like{i});
        shading('interp');
        hold on;
        if nargin>=2
          xy=range2xy(vis.angle,vis.range);
          plot(xy(obj.scanpts{i},1),xy(obj.scanpts{i},2),['w',sym{i}]);
        end
        plot(obj.prevlegs(i,1),obj.prevlegs(i,2),'wo');
        plot(obj.legs(i,1),obj.legs(i,2),'wx');
        title(sprintf('Leg %d',i));
        if min(obj.like{i})<-10
          caxis(max(obj.like{i}(:))+[-30,0]);
        else
          caxis(min(obj.like{i}(:))+[0,30]);
        end
        colorbar
        axis equal
        axis([min(obj.minval(:,1)),max(obj.maxval(:,1)),min(obj.minval(:,2)),max(obj.maxval(:,2))]);
      end
      if nargin>=2
        suptitle(sprintf('ID %d Frame %d',obj.id,vis.frame));
      else
        suptitle(sprintf('ID %d',obj.id));
      end
    end 


    % OBSOLETE
    function oldupdate(obj, vis, i,j, nstep,fps);
    % Update positions of object using vis with legs assigned to classes i,j
      params=getparams();
      obj.legclasses=[i,j];
      if i~=1
        obj.legs(1,:)=obj.circmodel(vis,i,true);
      end
      if j~=1
        obj.legs(2,:)=obj.circmodel(vis,j,true);
      end
      if isempty(fs{1})
        obj.legs(1,:)=obj.nearestshadowed(vis,obj.legs(2,:),params.maxlegsep+obj.legdiam,obj.legs(1,:));
      end
      if j==1
        obj.legs(2,:)=obj.nearestshadowed(vis,obj.legs(1,:),params.maxlegsep+obj.legdiam,obj.legs(2,:));
      end
      if i~=1
        obj.posvar(1)=params.initialPositionVar;   % Locked down again
      end
      if j~=1
        obj.posvar(2)=params.initialPositionVar;
      end
      
      % DELETED stuff to do updates here and moved to new update()
    end

    % OBSOLETE
    function pos=nearestshadowed(obj,vis,otherlegpos,maxdist,targetpos)
    % Estimate position of a circle (leg) of diameter obj.legdiam, that is fully shadowed
    % Must be within maxdist of otherlegpos
    % Should also be as close to targetpos as possible
      pos=targetpos;
      if norm(otherlegpos-pos) > maxdist
        if obj.debug
          fprintf('Target position (%.2f,%.2f) is too far (%.2fm) from other leg at (%.2f,%.2f). ',pos, norm(pos-otherlegpos), otherlegpos);
        end
        dir=pos-otherlegpos;  dir=dir/norm(dir);
        % Move as far as we can from other leg
        pos=otherlegpos+dir*maxdist;
        if obj.debug
          fprintf(' Moved to (%.2f,%.2f)\n', pos);
        end
      end
      
      if norm(otherlegpos-pos) < obj.legdiam
        if obj.debug
          fprintf('Target position (%.2f,%.2f) is too close (%.2fm) to other leg at (%.2f,%.2f). ',pos, norm(pos-otherlegpos), otherlegpos);
        end
        dir=pos-otherlegpos;  dir=dir/norm(dir);
        % Move as far as we can from other leg
        pos=otherlegpos+dir*obj.legdiam;
        if obj.debug
          fprintf(' Moved to (%.2f,%.2f)\n', pos);
        end
      end
      
      [theta,range]=xy2range(pos);
      anglewidth=obj.legdiam/range;
      if theta+anglewidth/2<vis.angle(1) || theta-anglewidth/2>vis.angle(end)
        % Already outside FOV, so just use pos
        return;
      end

      [~,cpos1]=min(abs(vis.angle-theta+anglewidth/2));
      [~,cpos2]=min(abs(vis.angle-theta-anglewidth/2));
      if all(vis.range(cpos1:cpos2)<range) || theta>pi/2 || theta<-pi/2
        % Already shadowed
        return;
      end
      
      % Current prediction is not shadowed
      % Find shadows that can hide this object
      % ledge,redge,srange gives left, right indices, distance of each shadow
      minrange=max(range,vis.range+obj.legdiam/2);
      minrange=minrange(:)';
      srange=minrange;
      ledges=1:length(srange);
      redges=ledges;
      res=vis.angle(2)-vis.angle(1);
      while true
        % Compute widths of each shadow
        widths=(redges-ledges+2).*srange*res;
        % Check if big enough
        if all(widths>=obj.legdiam)
          break;
        end
        % Expand small ones
        redges(widths<obj.legdiam)=redges(widths<obj.legdiam)+1;
        % Last possibility may be past end, remove it
        sel=redges<=length(srange);
        redges=redges(sel);
        ledges=ledges(sel);
        srange=srange(sel);
        srange=max(srange(ledges),minrange(redges));
      end

      scenters=range2xy((vis.angle(ledges)+vis.angle(redges))/2,srange);
      cdist=sqrt((scenters(:,1)-pos(1)).^2+(scenters(:,2)-pos(2)).^2);
      [~,minpos]=min(cdist);
      ledge=ledges(minpos);
      redge=redges(minpos);
      pos=scenters(minpos,:);
      distmoved=norm(pos-targetpos);
      if obj.debug
        fprintf('Best location for (%.2f,%.2f) is in shadow %d-%d at (%.2f,%.2f) with a distance of %.2f\n', targetpos, ledge,redge,pos,distmoved);
      end
    end

    % OBSOLETE
    function pos=circmodel(obj,vis,class,update)
    % Estimate position of a circle (leg) of diameter obj.legdiam, that results in echoes in vis for class 
      sel=[vis.class]==class;
      fsel=find(sel);
      assert(sum(sel)>0);

      res=vis.angle(2)-vis.angle(1);
      maxangle=max(vis.angle(sel));
      minangle=min(vis.angle(sel));
      theta=maxangle-minangle+res;
      if sum(sel)==1
        % Just one point 
        % Sometimes scan will average two nearby targets, thus making the mostly shadowed edge appear too close
        range=vis.range(fsel)+obj.legdiam/4;
        if fsel>1 && vis.range(fsel-1)<vis.range(fsel)
          range=max(range,vis.range(fsel-1)+obj.legdiam*5/4);
        end
        if fsel<length(vis.range) && vis.range(fsel+1)<vis.range(fsel)
          range=max(range,vis.range(fsel+1)+obj.legdiam*5/4);
        end
      else
        range=mean(vis.range(sel))+obj.legdiam/4;
      end
      
      % Set position of leg
      anglewidth=obj.legdiam/range;
      shadowed=vis.shadowed(sel,:);
      if shadowed(1,1)&~shadowed(end,2)
        angle=maxangle+res/2-anglewidth/2;
      elseif ~shadowed(1,1)&shadowed(end,2)
        angle=minangle-res/2+anglewidth/2;
      elseif shadowed(1,1)&shadowed(end,2)
        if obj.debug
          fprintf('Person %d, both edges shadowed\n', obj.id);
        end
        angle=(maxangle+minangle)/2;
      else
        % Unshadowed
        r=range*theta;
        if update
          if abs(r-obj.legdiam)>0.05 && obj.debug
            fprintf('Person %d legdiam was %.2f, current observation=%.2f\n', obj.id, obj.legdiam, r);
          end
          obj.legdiam=obj.legdiam*0.99+r*0.01;
        end
        angle=(maxangle+minangle)/2;
      end
      pos=range2xy(angle,range);
    end
  end
end