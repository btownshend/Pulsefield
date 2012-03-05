function v=calcvisible(cpos,cdir,fov,lpos,tpos,tgtdiam)
col=1-0.3*[0 1 0
           1 0 0
           0 0 1
           1 0 1
           1 1 0
           0 1 1
           0.5 1 1
           1 0.5 1
           1 1 0.5
           0.5 0.5 1
           0.5 1 0.5
           1 0.5 0.5
          ];
for l=1:size(lpos,1)
  for c=1:size(cpos,1)
    vdir=lpos(l,:)-cpos(c,:);
    angle=acos(dot(vdir,cdir(c,:))/norm(vdir)/norm(cdir(c,:)));
    
    npts=100;
    ray=[];
    ray(:,1)=(0:npts)/npts*(lpos(l,1)-cpos(c,1))+cpos(c,1);
    ray(:,2)=(0:npts)/npts*(lpos(l,2)-cpos(c,2))+cpos(c,2);

    if angle>fov/2
      % Not in FOV, unknown visibility
      v(c,l)=nan;
%      plot(ray(:,1),ray(:,2),'Color',0.78*[1,1,1])
    else
      v(c,l)=1;
      for t=1:size(tpos,1)
        d=(ray(:,1)-tpos(t,1)).^2+(ray(:,2)-tpos(t,2)).^2;
        strikept=min(find(d<(tgtdiam(t)/2).^2));
        if ~isempty(strikept)
          % Blocked view
          v(c,l)=0;
          ray=ray(1:strikept,:);	% only keep first part of ray for subsequent target hunting
        end
      end
      if v(c,l)==0
        % Plot remaining part of ray (from camera to first hit target)
        plot(ray(:,1),ray(:,2),'Color',col(c,:));
      end
    end
  end
end
