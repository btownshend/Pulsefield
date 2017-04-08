  cbounds=[-1 -1 -1
           -1 1 -1
           1 1 -1
           1 -1 -1
           -1 -1 -1];
  cbounds=[cbounds;cbounds];
  cbounds(6:end,3)=-cbounds(6:end,3);
  for k=1:3
    plot3(cbounds(:,mod(k,3)+1),cbounds(:,mod(k+1,3)+1),cbounds(:,mod(k+2,3)+1),'-');
    pause(1);
  end
