% Preparing the angles and global variables

N = [0 0 1]


%% Drawing the normal axis and view vector.
tt = tiledlayout(3,4);  % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';
title(tt,'Visualization of Cook-Torrance Specular BRDF');
% nexttile

%{
%% Visualize incident angles
draw_axis();
hold on
axis equal
i_ang_theta = linspace(-pi/2+0.1, pi/2-0.1, 10);
i_ang_phi = linspace(-pi/2,pi/2, 10);% -pi/2:0.1:pi/2; % This can be set as well for multi deminsional visualization
for theta = i_ang_theta
   for phi = i_ang_phi
       L = [sin(theta).*cos(phi) sin(theta).*sin(phi) cos(theta)];
        vectarrow([0 0 0], L);
        hold on
        axis equal
   end
end
%}
% Next plot



%% Visualize vectors with weights using Cook-Torrance Model.
% Preparing the Plot layout to draw on.
i_ang_theta = linspace(-pi/2+0.1, pi/2-0.1, 10);
i_ang_phi = linspace(-pi/2,pi/2, 10);% -pi/2:0.1:pi/2; 

v_ang_theta = deg2rad(45); % linspace(deg2rad(45), deg2rad(90), 2);
v_ang_phi = deg2rad(0);
roughness = linspace(0.2, 1.0, 3);
n = 1.576;

%{%}


for v_ang = v_ang_theta
    V = [sin(v_ang).*cos(v_ang_phi) sin(v_ang).*sin(v_ang_phi) cos(v_ang)]
    % vectarrow(V.*2, [0 0 0]);
    axis equal
    for r = roughness
        draw_cook_vectors(V, N, i_ang_theta, i_ang_phi, r, n);
        disp("A Diagram completed");
    end
    hold off
end



