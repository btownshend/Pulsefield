
if ~exist('n_ima'),
   fprintf(1,'No data to process.\n');
   return;
end;

if n_ima == 0,
    fprintf(1,'No image data available\n');
    return;
end;

if ~exist('active_images'),
	active_images = ones(1,n_ima);
end;
n_act = length(active_images);
if n_act < n_ima,
   active_images = [active_images ones(1,n_ima-n_act)];
else
   if n_act > n_ima,
      active_images = active_images(1:n_ima);
   end;
end;

ind_active = find(active_images);

% I did not call check_active_images, because I want to prevent a break
%check_active_images;
   
   if ~isempty(ind_active),
      
      if length(ind_active) > 2,
      
   		for ii = 1:length(ind_active)-2,
      		
         	fprintf(1,'%d, ',ind_active(ii));
         	
      	end;
      	
      	fprintf(1,'%d and %d.',ind_active(end-1),ind_active(end));
         
      else
         
         if length(ind_active) == 2,
            
            fprintf(1,'%d and %d.',ind_active(end-1),ind_active(end));
            
         else
            
            fprintf(1,'%d.',ind_active(end));
            
         end;
         
         
      end;
      
   end;
      
% force image suppression
choice = 0;


if isempty(ima_numbers),
  fprintf(1,'No image has been suppressed. No modication of the list of active images.\n',n_ima);
ima_proc = [];
else
ima_proc = ima_numbers;
end;


if ~isempty(ima_proc),
   
   active_images(ima_proc) = choice * ones(1,length(ima_proc));
   
end;


   check_active_images;
   

   fprintf(1,'\nThere are now a total of %d active images for calibration:\n',length(ind_active));
   
   if ~isempty(ind_active),
      
      if length(ind_active) > 2,
      
   		for ii = 1:length(ind_active)-2,
      		
         	fprintf(1,'%d, ',ind_active(ii));
         	
      	end;
      	
      	fprintf(1,'%d and %d.',ind_active(end-1),ind_active(end));
         
      else
         
         if length(ind_active) == 2,
            
            fprintf(1,'%d and %d.',ind_active(end-1),ind_active(end));
            
         else
            
            fprintf(1,'%d.',ind_active(end));
            
         end;
         
         
      end;
      
   end;
   
   
   
