% Test level controls over camera
function testlevcontrols(id)
ds=1000;   % Downsample factor
satthresh=250;
v={struct('autoexp','on','exposure','on','brightness',0,'lowlight','quality','shortexposures',1,'analoggain',1,'maxexptime',1),
   struct('autoexp','on','exposure','on','brightness',0,'lowlight','quality','shortexposures',1,'analoggain',1,'maxexptime',100),
   struct('autoexp','on','exposure','on','brightness',0,'lowlight','quality','shortexposures',1,'analoggain',1,'maxexptime',1)};
n=length(v);
s={};x={};
for i=1:n
  s{i}=setcam(id,v{i});
  x{i}=arecont(id);
  for j=1:3
    ic=x{i}.im(:,:,j);
    ic=sort(ic(:));
    x{i}.ic{j}=ic(1:ds:end);
    x{i}.sat(j)=sum(ic>=satthresh)/length(ic);
    x{i}.med(j)=ic(round(length(ic))/2);
    x{i}.mid(j)=sum(ic>=128)/length(ic);
  end
  fprintf('%s: pct(>=%d)=(%.2f%%,%.2f%%,%.2f%%), pct(>=128)=(%.2f%%,%.2f%%,%.2f%%), median=(%d,%d,%d)\n', s{i}, satthresh, 100*x{i}.sat, 100*x{i}.mid,x{i}.med);
end

figure;
col='rgb';
for i=1:n-1
  subplot(n,n,1+n*i);
  for j=1:3
    h=cdfplot(x{i}.ic{j});
    set(h,'Color',col(j));
    hold on;
  end
  xlabel(sprintf('x (sat=%.2f%%,%.2f%%,%.2f%%)',100*x{i}.sat));
  title(s{i});
end

for i=2:n
  subplot(n,n,i);
  for j=1:3
    h=cdfplot(x{i}.ic{j});
    set(h,'Color',col(j));
    hold on;
  end
  xlabel(sprintf('x (sat=%.2f%%,%.2f%%,%.2f%%)',100*x{i}.sat));
  title(s{i});
end

for k=1:n
  for i=k+1:n
    ti=['Compare(',s{i},',',s{k},')'];
    subplot(n,n,i+n*k);
    for j=1:3
      plot(x{i}.ic{j},x{k}.ic{j});
      hold on;
    end
    plot([0,255],[0,255],':k');
    xlabel(s{i});
    ylabel(s{k});
    title('Compare');
  end
end
suptitle(sprintf('Camera %d',id));    

function s=setcam(id,settings)
fn=fieldnames(settings);
s='';
for i=1:length(fn)
  val=settings.(fn{i});
  arecont_set(id,fn{i},val);
  s=[s,fn{i}(1:2),'='];
  if isnumeric(val)
    s=[s,num2str(val),','];
  else
    s=[s,sprintf('%0.2s',val),','];
  end
end
s=s(1:end-1);
pause(5);
