% plot the trail of people
function plothypo(recvis)
snap=recvis.snap;
t=(arrayfun(@(z) z.when, snap)-snap(1).when)*24*3600;
occ=arrayfun(@(z) length(z.hypo), snap);

setfig('plothypo');
clf;
subplot(221);
plotlayout(recvis.p.layout,0);
hold on;
for samp=1:length(snap)
  foundk=zeros(length(snap(samp).hypo),1);

  if samp>1
    [ids,pid,cid]=intersect([snap(samp-1).hypo.id],[snap(samp).hypo.id]);
    for j=1:length(ids)
      hj=snap(samp-1).hypo(pid(j));
      hk=snap(samp).hypo(cid(j)); 
      plot([hj.pos(1),hk.pos(1)],[hj.pos(2),hk.pos(2)],'Color',id2color(ids(j),recvis.p.colors));
    end

    % Exits
    [~,exid]=setdiff([snap(samp-1).hypo.id],[snap(samp).hypo.id]);
    for j=1:length(exid)
      hj=snap(samp-1).hypo(exid(j));
      plot(hj.pos(1),hj.pos(2),'x','Color',id2color(hj.id,recvis.p.colors));
    end

    % Entrances
    [~,enid]=setdiff([snap(samp).hypo.id],[snap(samp-1).hypo.id]);
    for j=1:length(enid)
      hj=snap(samp).hypo(enid(j));
      plot(hj.pos(1),hj.pos(2),'o','Color',id2color(hj.id,recvis.p.colors));
    end
  end
end

% Initial entres
for j=1:length(snap(1).hypo)
  hj=snap(1).hypo(j);
  plot(hj.pos(1),hj.pos(2),'o','Color',id2color(hj.id,recvis.p.colors));
end
% Final exits
for j=1:length(snap(end).hypo)
  hj=snap(end).hypo(j);
  plot(hj.pos(1),hj.pos(2),'x','Color',id2color(hj.id,recvis.p.colors));
end
  

subplot(222);
hold on;
for samp=2:length(snap)
  [ids,pid,cid]=intersect([snap(samp-1).hypo.id],[snap(samp).hypo.id]);
  for j=1:length(snap(samp).tgts)
    tj=snap(samp).tgts(j);
    plot(([samp,samp]),[min(tj.pixellist(:,1)),max(tj.pixellist(:,1))],'k'); 
    plot((samp),tj.pos(1),'.k');
  end
  for j=1:length(ids)
    hj=snap(samp-1).hypo(pid(j));
    hk=snap(samp).hypo(cid(j));
    plot(([samp-1,samp]),[hj.pos(1),hk.pos(1)],'Color',id2color(hj.id,recvis.p.colors));
  end
end
xlabel('Sample');
ylabel('X Position (m)');

subplot(224);
hold on;
for samp=2:length(snap)
  [ids,pid,cid]=intersect([snap(samp-1).hypo.id],[snap(samp).hypo.id]);
  for j=1:length(snap(samp).tgts)
    tj=snap(samp).tgts(j);
    plot(([samp,samp]),[min(tj.pixellist(:,2)),max(tj.pixellist(:,2))],'k');
    plot((samp),tj.pos(2),'.k');
  end
  for j=1:length(ids)
    hj=snap(samp-1).hypo(pid(j));
    hk=snap(samp).hypo(cid(j));
    plot(([samp-1,samp]),[hj.pos(2),hk.pos(2)],'Color',id2color(hj.id,recvis.p.colors));
  end
end
xlabel('Sample');
ylabel('Y Position (m)');

subplot(223)
plot(t,occ);
xlabel('Time (sec)');
ylabel('Occupancy');
title(sprintf('%.1f fps', length(snap)/t(end)));

if isfield(recvis,'note')
  suptitle(recvis.note);
end
