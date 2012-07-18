function plottrack(tgts,hypo)  
setfig('analyze.estimates');
hold on;
tpos=nan(length(tgts),max(cellfun(@(z) size(z.tpos,1),tgts)),2);
for i=1:length(tgts)
  n=size(tgts{i}.tpos,1);
  tpos(i,1:n,:)=tgts{i}.tpos;
end
for j=1:size(tpos,2)
  t=squeeze(tpos(:,j,:));
  plot(t(:,1),t(:,2),'b-');
  plot(t(end,1),t(end,2),'bo');
end
hpos=[];
for i=1:length(hypo)
  n=size(hypo{i}.pos,1);
  hpos(i,1:n,:)=hypo{i}.pos;
end
for j=1:size(hpos,2)
  h=squeeze(hpos(:,j,:));
  plot(h(:,1),h(:,2),'g-');
  plot(h(end,1),h(end,2),'gx');
end


