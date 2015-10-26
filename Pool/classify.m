directory = 'sets';
files = dir(directory);

calibration=load('arecont/C2.fisheye/c2-Results_left.mat');

if exist('SVMModel.mat','file')
    data = load('SVMModel');
    SVMModel = data.SVMModel;
else
    
    all = {};
    
    for i=1:length({files.name})
        if ~isempty(strfind(files(i).name, 'cues'))
            all{length(all) + 1} = [directory '/' files(i).name];
        end
        
        if ~isempty(strfind(files(i).name, 'balls'))
            all{length(all) + 1} = [directory '/' files(i).name];
        end
        
        if ~isempty(strfind(files(i).name, 'background'))
            all{length(all) + 1} = [directory '/' files(i).name];
        end
    end
    
    allExamples = [];
    allLabels = [];
    
    for i=1:length(all)
        raw = imread(all{i});
        
        red = raw(:,:,1);
        green = raw(:,:,2);
        blue = raw(:,:,3);
        redS = red(red ~= 128 & green ~= 128 & blue ~= 128);
        greenS = green(red ~= 128 & green ~= 128 & blue ~= 128);
        blueS = blue(red ~= 128 & green ~= 128 & blue ~= 128);
        selected = horzcat(redS, greenS, blueS);
        
        if ~isempty(strfind(all{i}, 'background'))
            labels = 1 * ones(length(selected), 1);
        end
        
        if ~isempty(strfind(all{i}, 'balls'))
            labels = 2 * ones(length(selected), 1);
        end
        
        if ~isempty(strfind(all{i}, 'cues'))
            labels = 2 * ones(length(selected), 1);
        end
        
        allExamples = vertcat(allExamples, selected);
        allLabels = vertcat(allLabels, labels);
    end
    
    numBack = length(allLabels(allLabels == 1));
    numBall = length(allLabels(allLabels == 2));
    
    rng(12313);
    reorder = randperm(length(allLabels));
    randomLabels = allLabels(reorder);
    randomExamples = allExamples(reorder,:);
    
    minClass = min([numBack, numBall]);
    minClass = 10000;
    
    fprintf('%d background pixels, %d ball/cue pixels\n', numBack, numBall, numCue);
    
    figure(1)
    backgrounds = find(randomLabels == 1, minClass);
    balls = find(randomLabels == 2, minClass);
    scatter3(randomExamples(backgrounds,1), randomExamples(backgrounds,2), randomExamples(backgrounds,3));
    hold on
    scatter3(randomExamples(balls,1), randomExamples(balls,2), randomExamples(balls,3));
    pruned = vertcat(backgrounds, balls, cues);
    
    prunedExamples = randomExamples(pruned, :);
    prunedLabels = randomLabels(pruned, :);
    
    
    numPruned = length(prunedLabels);
    numTrain = floor(numPruned * 0.5);
    numTest = numPruned - numTrain;
    
    reorder = randperm(length(prunedLabels));
    randomLabels = prunedLabels(reorder);
    randomExamples = prunedExamples(reorder,:);
    
    
    fprintf('%d training examples, %d testing examples\n', numTrain, numTest);
    
    labelsTrain = randomLabels(1:numTrain);
    labelsTest = randomLabels(numTrain+1:end);
    train = randomExamples(1:numTrain,:);
    test = randomExamples(numTrain+1:end,:);
    
    SVMModel = fitcsvm(double(train),labelsTrain);
    
    labelsPredict = predict(SVMModel, double(test));
    
    mat = confusionmat(labelsTest, labelsPredict)
    save('SVMModel.mat','SVMModel');
end

while(true)
    tic();
    curr=arecont(2);
    toc();
    
    
    half_im = curr.im;
    
    setfig('Original');
    imshow(half_im);
    pause(0.1);
    
    tic();
    toPredict = reshape(half_im, size(half_im,1) * size(half_im,2), size(half_im,3));
    [prediction, ~] = predict(SVMModel, double(toPredict));
    labeled_half_im = reshape(prediction - 1, size(half_im,1), size(half_im,2));
    toc();
    
    
    
    setfig('Labeled');
    imshow(labeled_half_im);
    pause(0.1);
    
    
    upsampled = imresize(labeled_half_im, 2);
    
    setfig('Rectified');
    
    tic();
    ar = rectify(upsampled,calibration,1);
    toc();
    
    rotation = -0.9832;
    rotated = imrotate(ar, rotation);
    
    tl = [775, 659];
    tl = tl + 54;
    br = [3628, 2153];
    br = br - 54;
    br(2) = br(2) - 33;
    
    imshow(rotated);
    hold on;
    plot([tl(1), tl(1), br(1), br(1), tl(1)], [tl(2), br(2), br(2), tl(2), tl(2)], 'r');
    pause(0.1);
    
    cropped = imcrop(rotated, [tl, br - tl]);
    
%     setfig('Cropped');
%     imshow(cropped);
%     pause(0.1);
    
    
    pockets_gone = cropped;
    pockets_gone(1:65,1:65) = 0;
    
    pockets_gone(end-90:end, 1:90) = 0;
    pockets_gone(1:60, end-80:end) = 0;
    pockets_gone(end-90:end, end-90:end) = 0;
    pockets_gone(end-15:end, 1340:1415) = 0;
    
    
%     setfig('Pockets_Gone');
%     imshow(pockets_gone);
%     pause(0.1);
    
    
    
    closed = imclose(pockets_gone,strel('disk',10));
%     setfig('Closed');
%     imshow(closed);
%     pause(0.1);
    
    opened = imopen(closed,strel('disk',5));
%     setfig('Opened');
%     imshow(opened);
%     pause(0.1);
    
    
    
    binarized = im2bw(opened, 0.5);
    setfig('Binarized');
    imshow(binarized);
    
    
    
    blobAnalyser = vision.BlobAnalysis('LabelMatrixOutputPort', true, 'BoundingBoxOutputPort', true, ...
        'AreaOutputPort', true, 'CentroidOutputPort', true, ...
        'MinimumBlobArea', 1000);
    
    [blobs, centroids, bboxes, labels] = blobAnalyser.step(binarized);
    
    hold on
    scatter(centroids(:,1), centroids(:,2), 'red');
    
    pause(0.1);
    
    image_center = size(binarized) ./ 2;
    image_center = image_center(2:-1:1);
    
    best_length = 0;
    best_blob = -1;
    best_angle = 0;
    best_center = [-1 -1];
    for i=1:length(blobs)
        setfig(sprintf('Edges-%d', i))
        BW = edge(labels == i,'sobel', 'nothinning');
        [H,T,R] = hough(BW);
        P  = houghpeaks(H,5,'threshold',ceil(0.01*max(H(:))));
        lines = houghlines(BW,T,R,P,'FillGap',30,'MinLength',2);
        imshow(BW);
        
        most_central = [0 0];
        
        len = 0;
        angle = [];
        hold on
        for k = 1:length(lines)
            xy = [lines(k).point1; lines(k).point2];
            plot(xy(:,1),xy(:,2),'LineWidth',2,'Color','green');
            
            % Plot beginnings and ends of lines
            plot(xy(1,1),xy(1,2),'x','LineWidth',2,'Color','yellow');
            plot(xy(2,1),xy(2,2),'x','LineWidth',2,'Color','red');
            
            angle = [angle atan2(xy(2,2) - xy(1,2), xy(2,1) - xy(1,1))];
            
            len = len + norm(lines(k).point1 - lines(k).point2);
            
            if norm(lines(k).point1 - image_center) < norm(most_central - image_center)
                most_central = lines(k).point1;
            end
            
            if norm(lines(k).point2 - image_center) < norm(most_central - image_center)
                most_central = lines(k).point2;
            end
        end
        
        scatter(most_central(1),most_central(2),'blue');
        
        angle = angle * 180 / pi;
        median_angle = median(angle);
        
        pause(0.1);
        fprintf('Blob %d area = %d, num lines = %d, angle = %f, length = %f, most_central = (%d, %d)\n', ...
            i, blobs(i), length(lines), median_angle, len, most_central);
        
        if len > best_length
            best_length = len;
            best_blob = i;
            best_angle = median_angle;
            best_center = most_central;
        end
        
    end
    
    if best_center(1) == -1 || best_length < 200
        fprintf('No cue\n');
        oscmsgout('PHYSICS','/nocue',{});
    else
        
        cue_center = (((best_center - [1 1]) ./ (image_center * 2)) - [0.5 0.5]) .* [-1.117, 0.560];
        cue_angle = best_angle;
        
        cue_vector = [cosd(cue_angle) sind(cue_angle)];
        
        fprintf('Cue location: (%f, %f), cue vector: (%f, %f), from blob %d\n', cue_center, cue_vector, best_blob);
        oscmsgout('PHYSICS','/cue',{cue_center(1), cue_center(2), cue_vector(1), cue_vector(2)});
    end
end

