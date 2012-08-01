function recanalyze(rec,index)
setfig('recanalyze');
clf;
if nargin<2
  % Do overall position track
  plotlayout(rec.p.layout);
  hold on;
  for i=1:length(rec.vis)
    [~,est]=analyze(rec.p,rec.vis(i).v,0);
    fprintf('Sample %d at %s:\n',i,datestr(rec.vis(i).when));
    plot(est.tpos(:,1),est.tpos(:,2),'.');
    text(est.tpos(:,1),est.tpos(:,2),num2str(i));
    pause(0.1);
  end
else
  % Analyze single image
  if isfield(rec.vis(index),'im')
    plotvisible(rec.p,rec.vis(index));
  end
  analyze(rec.p,rec.vis(index).v,2);
end
