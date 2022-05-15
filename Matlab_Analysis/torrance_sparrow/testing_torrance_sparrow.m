
angles = 0.0:0.1:(pi/2 );

%[x y] = meshgrid(angles); % Generate x and y data




%{
z = geometric_attenuation_torrance_sparrow(x,y);

%disp('The final result is: ')
%disp(z)
 %  % Generate z data
% surf(x, y, z) % Plot the surface 
%}

% plot3( [1 -1 -1 1] , [0 0 0 0], [1 1 -1 -1])
[x y] = meshgrid(angles,angles); % Creates a grid of values for x and y instead of similar ones.
z = zeros(size(x));


%% Get Geometric attenuation using Torrance Sparrow Model based on ANGLES not Vectors.
%{
for i = 1:size(x,1)
    for j = 1:size(y,1)
        z(i,j) = abstract_torrance_geom(y(i,j),x(i,j));
    end
end
surf(x.*180/pi, y*180/pi, z);
%}


%% Ploting points
% angles_2 = 0.0:0.4:(pi/2);
studied_inc = 10:10:80;
studied_ref = (-pi/2 + 0.1):0.1:(pi/2-0.1);
for incd = studied_inc*(pi/180)
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        plotted_geom(ri) = abstract_torrance_geom(incd, studied_ref(ri), true);
        % plotted_geom(ri) = plotted_geom(ri)/cos(studied_ref(ri));
    end
    % disp(plotted_geom)
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('Geometric Attenuations for Multiple Incident Angles');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end
hold off
%% Torrance-Sparrow Geometric Attenuation model based on vector arithmatics.
%{
for incd = studied_inc*(pi/180)
    plotted_geom = zeros(size(studied_ref));
    for ri = 1:size(studied_ref,2)
        plotted_geom(ri) = abstract_torrance_geom(incd, studied_ref(ri));
        % plotted_geom(ri) = plotted_geom(ri)/cos(studied_ref(ri));
    end
    % disp(plotted_geom)
    plot3(repelem(rad2deg(incd), size(studied_ref,2)), studied_ref.*180./pi, plotted_geom);
    title('Geometric Attenuations for Multiple Incident Angles');
    xlabel('Incident Angle');
    ylabel('Reflection Angle');
    zlabel('Geometric Factor');
    hold on
end
hold off


%}

