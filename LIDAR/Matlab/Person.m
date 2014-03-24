classdef Person < handle
  properties
    id; 	% ID of person
    position; 	% Position of core (hips)
    legs;   	% Coordinates of legs;  legs(1,:)=left, legs(2,:)=right
    prevlegs;	% Previous leg coordinates
    legvelocity;% Velocity of each leg
    legclasses;	% Classes assigned to legs
    posvar;	% Estimated variance of position of legs 
    velocity;   % Overall velocity
                %facing;  	% Vector pointing forward
    legdiam;	% Estimate of leg diameter in meters
    leftness;	% Fraction of time leg(1) is on the left side of direction of motion
    age;
    consecutiveInvisibleCount;
    totalVisibleCount;
    debug;
  end
  
  methods
    function obj=Person(id,vis,class1,class2,debug);
      if nargin<1
        return;   % For cloning
      end
      params=getparams();
      
      if nargin<5
        obj.debug=false;
      else
        obj.debug=debug;
      end
      
      if obj.debug
        if isempty(class2)
          fprintf('Creating person %d using class %d\n',id, class1);
        else
          fprintf('Creating person %d using classes %d,%d\n',id, class1,class2);
        end
      end
      obj.id=id;
      obj.legdiam=params.initlegdiam;

      obj.legs(1,:)=obj.circmodel(vis,class1,false);
      if ~isempty(class2)
        obj.legs(2,:)=obj.circmodel(vis,class2,false);
        obj.legclasses=[class1,class2];
      else
        % Assume 2nd leg is directly behind first one
        [theta,range]=xy2range(obj.legs(1,:));
        obj.legs(2,:)=range2xy(theta,range+obj.legdiam);
        obj.legclasses=[class1,1];
      end

      obj.prevlegs=obj.legs;
      obj.position=mean(obj.legs,1);
      obj.posvar=params.initialPositionVar*[1,1];
      obj.velocity=[0,0];
      obj.legvelocity=zeros(2,2);
      obj.leftness=0.0;
      obj.age=1;
      obj.consecutiveInvisibleCount=0;
      obj.totalVisibleCount=1;
      fprintf('Created %s\n',obj.tostring());
    end
    
    function s=tostring(obj)
      s=sprintf('P%d at (%.2f,%.2f) with legs at (%.2f,%.2f)[%d], (%.2f,%.2f)[%d]; vel=(%.2f,%.2f),(%.2f,%.2f) posstd=(%.2f,%.2f), age=%d,cic=%d,tvc=%d', obj.id, obj.position,obj.legs(1,:), obj.legclasses(1), obj.legs(2,:),obj.legclasses(2), obj.legvelocity(1,:), obj.legvelocity(2,:),sqrt(obj.posvar),obj.age,obj.consecutiveInvisibleCount,obj.totalVisibleCount);
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
      obj.prevlegs=obj.legs;
      for i=1:2
        obj.legs(i,:)=obj.legs(i,:)+obj.legvelocity(i,:)*nstep/fps;
      end
      obj.position=mean(obj.legs,1);
      obj.velocity=mean(obj.legvelocity,1);
      obj.posvar=obj.posvar+params.driftVar*nstep;	% Amount of drift per unit step
    end
    
    function [like,p1,p2]=getclasslike(obj,vis)
    % Compute like(i,j) which is likelihood that class i is leg1, class j is leg 2
    % like(i,1) and like(i,j) represents leg not being visible (shadowed)
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
      
    function update(obj, vis, i,j, nstep,fps);
    % Update positions of object using vis with legs assigned to classes i,j
      params=getparams();
      obj.legclasses=[i,j];
      if i~=1
        obj.legs(1,:)=obj.circmodel(vis,i,true);
      end
      if j~=1
        obj.legs(2,:)=obj.circmodel(vis,j,true);
      end
      if i==1
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
      newpos=mean(obj.legs,1);
      % Update velocity
      newlegvelocity=(obj.legs-obj.prevlegs)/(nstep/fps);
      obj.legvelocity=obj.legvelocity*(1-1/params.velupdatetc)+newlegvelocity/params.velupdatetc;
      if i==1
        obj.legvelocity(1,:)=obj.legvelocity(1,:)*params.veldamping;
      end
      if j==1
        obj.legvelocity(2,:)=obj.legvelocity(2,:)*params.veldamping;
      end
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

      delta=newpos-obj.position;
      obj.leftness=obj.leftness*0.99+0.01*dot([-delta(2),delta(1)],diff(obj.legs,1));
      if false && obj.leftness<0.0
        % TODO: this causes the tracks to flip too often
        fprintf('Person %d: swapping legs: leftness=%.2f\n', obj.id, obj.leftness);
        obj.legs=obj.legs([2,1],:);
        obj.legvelocity=obj.legvelocity([2,1],:);
        obj.leftness=-obj.leftness;
      end
      obj.position=newpos;
      if i==1 && j==1
        obj.consecutiveInvisibleCount=obj.consecutiveInvisibleCount+1;
      else
        obj.consecutiveInvisibleCount=0;
        obj.totalVisibleCount=obj.totalVisibleCount+1;
      end
      obj.age=obj.age+1;
      if obj.debug
        fprintf('Updated %s\n', obj.tostring());
      end
    end

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

      [~,cpos1]=min(abs(vis.angle-theta-anglewidth/2));
      [~,cpos2]=min(abs(vis.angle-theta+anglewidth/2));
      if all(vis.range(cpos1:cpos2)<range)
        % Already shadowed
        return;
      end
      
      % Current prediction is not shadowed
      fprintf('Person %d leg at (%.2f,%.2f) is invisible, but not shadowed\n',obj.id, pos);

      % Find shadow points that are within range
      srange=max(range,vis.range+obj.legdiam/2);
      sxy=range2xy(srange,vis.angle);
      dist=(sxy(:,1)-otherlegpos(:,1)).^2 + (sxy(:,2)-otherlegpos(:,2)).^2;
      possible=dist<=maxdist+obj.legdiam/2;
      ledges=find([0;~possible(1:end-1)]&possible);
      redges=find(possible & [~possible(2:end);0]);
      widths=(sxy(redges,1)-sxy(ledges,1)).^2+(sxy(redges,2)-sxy(ledges,2)).^2;
      epossible=find(widths>=obj.legdiam^2);
      
      if ~isempty(epossible)
        fprintf('Person %d could be in %d different shadows\n', obj.id, length(epossible));
        scenters=sxy(ledges(epossible),:)+sxy(redges(epossible),:);
        cdist=sqrt((scenters(:,1)-pos(1))^2+(scenters(:,2)-pos(2)).^2);
        [~,minpos]=min(cdist);
        pos=scenters(minpos,:);
        fprintf('Best location is in shadow %d-%d at (%.2f,%.2f)\n', ledges(epossible(minpos)),redges(epossible(minpos)),pos);
      else
        fprintf('No feasible shadow found for person %d\n', obj.id);
        pos=[nan,nan];
        return;
      end
      
      distmoved=norm(pos-t.targetpos);
      fprintf('Moving to closest valid point at (%.2f,%.2f) with a distance of %.2f\n', pos, distmoved);
      if distmoved>1
        fprintf('**** Moved more than 1m\n');
      end
    end

    function pos=circmodel(obj,vis,class,update)
    % Estimate position of a circle (leg) of diameter obj.legdiam, that results in echoes in vis for class 
      sel=[vis.class]==class;
      assert(sum(sel)>0);

      res=vis.angle(2)-vis.angle(1);
      maxangle=max(vis.angle(sel));
      minangle=min(vis.angle(sel));
      theta=maxangle-minangle+res;
      range=mean(vis.range(sel))+obj.legdiam/4;
      
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