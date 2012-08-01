% Convert image from RGB to grayscale using [.5 .25 .25] weighting
% (Since cameras are less sensitive to red)
function im=rgb2graywithweight(im,weight)
if isa(im,'uint8') && (nargin<2 || (weight(1)==.5 && weight(2)==.25 && weight(3)==.25))
  % Faster
  im=bitshift(im(:,:,1),-1)+bitshift(im(:,:,2),-2)+bitshift(im(:,:,3),-2);
else
  if nargin<2
    weight=[0.5 0.25 0.25];
  end
  im=im(:,:,1)*weight(1)+im(:,:,2)*weight(2)+im(:,:,3)*weight(3);
end