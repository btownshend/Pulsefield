function plotvisible(sainfo,vis,figname)
if nargin<3
  setfig('getvisible');
else
  setfig(figname);
end
clf;
ncol=(isfield(vis,'corr')|isfield(vis,'v'))+isfield(vis,'im')+isfield(vis,'refim')+isfield(vis,'lev');
if ~isfield(vis,'v')
  vis.v=vis.lev>127;
end
for i=1:size(vis.v,1)
  c=sainfo.camera(i).pixcalib;
  roi=sainfo.camera(i).roi;
  col=1;
  if isfield(vis,'refim')
    subplot(length(vis.refim),ncol,(i-1)*ncol+col);
    col=col+1;
    imshow(vis.refim{i});
    axis normal
    hold on;
    title(sprintf('Reference %d',sainfo.camera(i).id));
    pause(0.01);
    for j=1:length(c)
      if ~isnan(vis.v(i,j))
        if vis.v(i,j)
          plot(c(j).pos(1)-roi(1)+1,c(j).pos(2)-roi(3)+1,'og');
        else
          plot(c(j).pos(1)-roi(1)+1,c(j).pos(2)-roi(3)+1,'or');
        end
      end
    end
  end
  if isfield(vis,'im')
    subplot(length(vis.im),ncol,(i-1)*ncol+col);
    col=col+1;
    imshow(vis.im{i});
    axis normal
    hold on;
    title(sprintf('Camera %d',sainfo.camera(i).id));
    pause(0.01);
    for j=1:length(c)
      if ~isnan(vis.v(i,j))
        if vis.v(i,j)
          plot(c(j).pos(1)-roi(1)+1,c(j).pos(2)-roi(3)+1,'og');
        else
          plot(c(j).pos(1)-roi(1)+1,c(j).pos(2)-roi(3)+1,'or');
        end
      end
    end
  end
  if isfield(vis,'lev')
    subplot(size(vis.v,1),ncol,(i-1)*ncol+col);col=col+1;
    lev=double(vis.lev(i,:));
    lev(isnan(vis.v(i,:)))=nan;
    plot(lev,'g');
    hold on;
    if isfield(sainfo,'crosstalk')
      % Plot threshold
      plot(sainfo.crosstalk.thresh(i,:),'c');
      plot(sainfo.crosstalk.neighlev(i,:),'m');
    end
    % Plot off signals in red
    lev(vis.v(i,:)==1)=nan;
    plot(lev,'rx');
    ax=axis;
    ax(3)=0;ax(4)=260;
    axis(ax);
    legend('Signal','Threshold','Neighlev');
  end
  if isfield(vis,'corr')
    subplot(size(vis.v,1),ncol,(i-1)*ncol+col);
    col=col+1;
    corr=vis.corr(i,:);
    corr(isnan(vis.v(i,:)))=nan;
    plot(corr,'g');
    hold on;
    if isfield(vis,'mincorr')
      % Plot threshold, broken with NaNs for unused LEDs
      mc=corr*0+vis.mincorr;
      plot(mc,'c');
    end
    % Plot off signals in red
    corr(vis.v(i,:)==1)=nan;
    plot(corr,'rx');
    ax=axis;
    ax(3)=min(ax(3),0);ax(4)=1.0;
    axis(ax);
    legend('Correlation','Threshold');
  elseif isfield(vis,'v')   % .v only, no .corr
    subplot(size(vis.v,1),ncol,(i-1)*ncol+col);
    col=col+1;
    vtmp=vis.v(i,:);
    vtmp(isnan(vtmp))=0.5;
    plot(vtmp,'g');
    hold on;
    % Plot off signals in red
    vtmp( (vtmp~=0) & ([vtmp(2:end),0]~=0) & ([0,vtmp(1:end-1)]~=0) )=nan;
    plot(vtmp,'r');
    ax=axis;
    ax(3)=-0.1;ax(4)=1.1;
    axis(ax);
  end
  xlabel('LED');
  ylabel('Signal');
end
fprintf('Can use: checkcalibration(p,vis,[led list]) to check particular leds\n');