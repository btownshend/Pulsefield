% Flash full string between two levels to see if the change is visible
function flashtest(s1,l1,l2)
while true
  setled(s1,-1,l1*[1,1,1],1,1);show(s1);sync(s1);
  pause(.1);
  setled(s1,-1,l2*[1,1,1],1,1);show(s1);sync(s1);
  pause(.1);
end