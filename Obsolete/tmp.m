for iid=1:length(sainfo.camera)
  roi=sainfo.camera(iid).roi
  for i=1:length(sainfo.camera(iid).pixcalib)
    if isfinite(sainfo.camera(iid).pixcalib(i).diameter)
      pixelList=sainfo.camera(iid).pixcalib(i).pixelList;
      pixelList(:,1)=pixelList(:,1)-roi(1)+1;
      pixelList(:,2)=pixelList(:,2)-roi(3)+1;
      % Setup indices from pixellists
      sainfo.camera(iid).pixcalib(i).indices=...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1)],pixelList(:,2),pixelList(:,1));
      sainfo.camera(iid).pixcalib(i).rgbindices=[...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),1*ones(size(pixelList,1),1));...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),2*ones(size(pixelList,1),1));...
          sub2ind([roi(4)-roi(3),roi(2)-roi(1),3],pixelList(:,2),pixelList(:,1),3*ones(size(pixelList,1),1))]
      % Flag for quick decisions
     sainfo.camera(iid).pixcalib(i).valid = true;
    else
     sainfo.camera(iid).pixcalib(i).valid = false;
    end
  end
end
