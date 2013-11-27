p={recvis.p};
p{1}.camera(5).pixcalib(2)=p{1}.camera(5).pixcalib(1);   % Bad pix data here
for i=1:3
  if i>1
    summary{i}=camdistcheck(p{i});
  end
  avglpos=summary{i}(1).ledpos;
  avglpos(:)=nanmean([[summary{i}(1).ledpos(:)],[summary{i}(2).ledpos(:)],[summary{i}(3).ledpos(:)],[summary{i}(4).ledpos(:)],[summary{i}(5).ledpos(:)],[summary{i}(6).ledpos(:)]],2);
  setfig(sprintf('lposerr%d',i));
  subplot(311);
  lerr=avglpos(:,1:2)-p{i}.layout.lpos;
  lerr(:,3)=avglpos(:,3);
  plot(lerr(:,1));
  ylabel('X error');
  subplot(312);
  plot(lerr(:,2));
  ylabel('Y error');
  xlabel('LED number');
  subplot(313);
  plot(lerr(:,3));
  ylabel('Z error');
  xlabel('LED number');
  suptitle(sprintf('LED Position error round %d',i));

  setfig(sprintf('overall error%d',i));
  subplot(311);
  bar([summary{i}.thetaerr]);
  title('Theta Error');
  subplot(312);
  bar([summary{i}.phierr]);
  title('Phi Error');
  subplot(313);
  bar([summary{i}.pixerr]);
  title('Sensor Pixel Error');
  suptitle(sprintf('Overall error round %d',i));

  % Corrected positions
  p{i+1}=p{i};
  p{i+1}.layout.lpos=avglpos(:,1:2);
  for j=1:6
    p{i+1}.layout.cpos(j,:)=summary{i}(j).campos(1:2);
  end
end