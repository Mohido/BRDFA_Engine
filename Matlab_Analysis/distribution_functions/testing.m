%% Blinn Distribution Function
middle_angles = (-pi/2 + 0.1):0.02:(pi/2-0.1); % (-pi/2 + 0.1):0.1:(pi/2-0.1);
roughness = 0.5:0.1:1.0;

tt = tiledlayout(3,2);  % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';
title(tt,'Distribution Functions');
nexttile

for r = roughness
    plotted_geom = zeros(size(middle_angles));
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = blinn(h, [0 1] , r);
    end
    plot3(repelem(r, size(middle_angles, 2)), middle_angles.*180./pi, plotted_geom);
    title('Blinn Distribution Function');
    xlabel('Roughness Value');
    ylabel('Half Vector Angle (H)');
    zlabel('Coeffecients');
    hold on
end
hold off
nexttile

for r = roughness
    plotted_geom = zeros(size(middle_angles,2));
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = blinn(h, [0 1] , r);
    end
    plot(middle_angles.*180./pi, plotted_geom);
    title('Blinn Distribution Function');    
    xlabel('Half Vector Angle (H)');
    ylabel('Coeffecients');
    grid on
    grid minor
    hold on
end
hold off
nexttile

%% Beckmann Distribution

for r = roughness
    plotted_geom = zeros(size(middle_angles));
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = beckmann(h, [0 1] , r);
    end
    plot3(repelem(r, size(middle_angles, 2)), middle_angles.*180./pi, plotted_geom);
    title('Beckmann Distribution Function');
    xlabel('Roughness Value');
    ylabel('Half Vector Angle (H)');
    zlabel('Coeffecients');
    hold on
end
hold off
nexttile

for r = roughness
    plotted_geom = zeros(size(middle_angles,2));
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = beckmann(h, [0 1] , r);
    end
    plot(middle_angles.*180./pi, plotted_geom);
    title('Beckmann Distribution Function');
    xlabel('Half Vector Angle (H)');
    ylabel('Coeffecients');
    grid on
    grid minor
    hold on
end
hold off
nexttile

%% Trowbridge-Reitz Distribution
for r = roughness
    plotted_geom = zeros(size(middle_angles));
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = trowbridge(h, [0 1] , r);
    end
    plot3(repelem(r, size(middle_angles, 2)), middle_angles.*180./pi, plotted_geom);
    title('Trowbridge GGX Distribution Function');
    xlabel('Roughness Value');
    ylabel('Half Vector Angle (H)');
    zlabel('Coeffecients');
    hold on
end
hold off
nexttile

for r = roughness
    plotted_geom = zeros(size(middle_angles,2));
    for mai = 1:size(middle_angles,2)   
        h = [cos(middle_angles(mai) + pi/2) sin(middle_angles(mai) + pi/2)];
        plotted_geom(mai) = trowbridge(h, [0 1] , r);
    end
    plot(middle_angles.*180./pi, plotted_geom);
    title('Trowbridge GGX Distribution Function');
    xlabel('Half Vector Angle (H)');
    ylabel('Coeffecients');
    grid on
    grid minor
    hold on
end
hold off







