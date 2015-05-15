function [corners,minerr,cornerAngle,orient]=cornerfit(pts)
% Find best split
minerr=1e10;
corners=[];
for i=3:size(pts,1)-2
  [fit1,e1]=orthfit(pts(1:i-1,1),pts(1:i-1,2));
  [fit2,e2]=orthfit(pts(i:end,1),pts(i:end,2));
  if e1+e2<minerr
    minerr=e1+e2;
    inter=-(fit2(2)-fit1(2))/(fit2(1)-fit1(1));
    inter(2)=fit2(1)*inter+fit2(2);
    verify=fit1(1)*inter+fit1(2);
    if abs(inter(2)-verify)>1e-5
      keyboard
    end
    corners=[pts(1,1),fit1(1)*pts(1,1)+fit1(2)
             inter
             pts(end,1),fit2(1)*pts(end,1)+fit2(2)];
    %    fprintf('slopes=[%.2f,%.2f] prod=%f, angle=%.0f deg\n',fit1(1),fit2(1),fit2(1)*fit1(1),atand(fit2(1)*fit1(1))*2);
    v1=corners(1,:)-corners(2,:);
    a1=mod(cart2pol(v1(1),v1(2))*180/pi+360,360);
    v2=corners(3,:)-corners(2,:);
    a2=mod(cart2pol(v2(1),v2(2))*180/pi+360,360);
    a3=mod(cart2pol(corners(2,1),corners(2,2))*180/pi+360,360);
    cornerAngle=mod(a1-a2+360,360);
    cornerAngle=min(cornerAngle,360-cornerAngle);
    orient=mod((a1+a2)/2-a3+360,360);
    orient=min(orient,360-orient);
    %    fprintf('a1=%.0f, a2=%.0f, a3=%.0f, angle=%.0f deg\n', a1, a2, a3, cornerAngle);
  end
end
minerr=sqrt(minerr/size(pts,1));
