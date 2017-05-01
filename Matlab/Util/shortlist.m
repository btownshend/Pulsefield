% Give short string to describe list with contiguous entries
function l=shortlist(x)
x=x(:)';
l='';
if isempty(x)
  l='[]';
  return;
end
x=sort(x);
bks=[1,find(diff(x)~=1)+1];
for i=1:length(bks)
  if i>1
    l=[l,','];
  end
  if i<length(bks) && bks(i+1)~=bks(i)+1
    l=[l,sprintf('%d-%d',x(bks(i)),x(bks(i+1)-1))];
  elseif i==length(bks) && length(x)>bks(i)
    l=[l,sprintf('%d-%d',x(bks(i)),x(end))];
  else
    l=[l,sprintf('%d',x(bks(i)))];
  end
end
