function plotconxs(x)
if ~isstruct(x)
  x=loadosc(x);
end
if isfield(x,'conx')
  cattr=fieldnames(x.conx);
else
  cattr={};
end
if isfield(x,'attr')
  attr=fieldnames(x.attr);
else
  attr={};
end
nplot=length(cattr)+length(attr);subnum=1;
if nplot==0
  fprintf('No connections in %s\n', x.filename);
  return;
end

setfig(['plotconxs-',x.filename]);clf;
col='rgbcmyk';
for i=1:length(cattr)
  subplot(nplot,1,subnum); subnum=subnum+1;
  cids={};
  pnum=1;
  for j=1:size(x.conx,2)
    for k=1:size(x.conx,3)
      cid='';gv=[];frame=[];cnt=0;
      for m=1:size(x.conx,1)
        gg=x.conx(m,j,k).(cattr{i});
        if ~isempty(gg)
          gv=[gv,gg.value];
          cid=gg.cid;
          cnt=cnt+1;
          frame=[frame,x.frame(m)];
        elseif ~isempty(frame) && x.frame(m)>frame(end)+10
          gv=[gv,nan];
          frame=[frame,x.frame(m)];
        end
      end
      if ~isempty(cid)
        fprintf('Conx attribute %s, CID %s has %d entries\n', cattr{i}, cid,cnt);
        plot(frame,gv,[col(mod(pnum-1,length(col))+1),'-']);
        hold on; 
        pnum=pnum+1;
        cids{end+1}=cid;
      end
    end
  end
  cids=unique(cids);
  ti=sprintf('%s: ',cattr{i});
  for j=1:length(cids)
    ti=[ti,sprintf('%s ', cids{j})];
  end
  title(ti,'Interpreter','none');
  ylabel('Value');
  c=axis;
  frames=double(x.frame(x.frame>0));
  axis([min(frames),max(frames),min(c(3),-.01),max(c(4),1.01)]);
end
for i=1:length(attr)
  subplot(nplot,1,subnum); subnum=subnum+1;
  uids=[];
  pnum=1;
  for j=1:size(x.attr,2)
    uid=-1;gv=[];frame=[];cnt=0;
    for m=1:size(x.attr,1)
      gg=x.attr(m,j).(attr{i});
      if ~isempty(gg)
        gv=[gv,gg.value];
        uid=gg.uid;
        cnt=cnt+1;
        frame=[frame,x.frame(m)];
      elseif ~isempty(frame) && x.frame(m)>frame(end)+10
        gv=[gv,nan];
        frame=[frame,x.frame(m)];
      end
    end
    if uid>0
      fprintf('Attribute %s, UID %s has %d entries\n', attr{i}, uid,cnt);
      plot(frame,gv,[col(mod(pnum-1,length(col))+1),'-']);
      hold on; 
      pnum=pnum+1;
      uids(end+1)=uid;
    end
  end

  for j=1:length(uids)
    uid=uids(j);
    [a,b]=find([x.uid]'==uid,1);
    firstframe=double(x.frame(b));
    [a,b]=find([x.uid]'==uid,1,'last');
    lastframe=double(x.frame(b));
    plot(firstframe*[1,1],[0,1],[col(mod(j-1,length(col))+1),':']);
    plot(lastframe*[1,1],[0,1],[col(mod(j-1,length(col))+1),':']);
    fprintf('UID %d: frames %d-%d\n', uid, firstframe,lastframe);
  end
  
  ti=sprintf('%s: ',attr{i});
  ti=[ti,sprintf('%d ', uids)];
  title(ti,'Interpreter','none');
  ylabel('Value');
  c=axis;
  frames=double(x.frame(x.frame>0));
  axis([min(frames),max(frames),min(c(3),-0.01),max(c(4),1.01)]);
end
xlabel('Frame');
suptitle(x.filename);
