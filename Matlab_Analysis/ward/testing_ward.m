% Preparing the angles and global variables


v_ang_theta = deg2rad(0);
v_ang_phi = deg2rad(0);

V = [sin(v_ang_theta).*cos(v_ang_phi), 
     sin(v_ang_theta).*sin(v_ang_phi),
     cos(v_ang_theta)
     ];
N = [0 0 1]

alpha =  0.15; % linspace(0.1, 0.8, 2);
beta = alpha; %linspace(0.1, 0.8, 2); % to consider Isotropic surfaces
rho = linspace(0.1, 1.0, 3); % 1.0;

% Preparing the Plot layout to draw on.
tt = tiledlayout(2,2);  % Defines 2by2 layout. 
tt.Padding = 'compact';
tt.TileSpacing = 'compact';
title(tt,'Visualization of Ward Distribution');
nexttile
 

%% Drawing the normal axis and view vector.
draw_axis();
vectarrow(V.*2, [0 0 0]);
hold on
axis equal

%% Visualize incident angles

i_ang_theta = linspace(-pi/2+0.1, pi/2-0.1, 10);
i_ang_phi = linspace(-pi/2,pi/2, 10);% -pi/2:0.1:pi/2; % This can be set as well for multi deminsional visualization

for theta = i_ang_theta
   for phi = i_ang_phi
       L = [sin(theta).*cos(phi), 
            sin(theta).*sin(phi),
            cos(theta)
            ];
        vectarrow([0 0 0], L);
        hold on
        axis equal
   end
end

% Next plot



%% Visualize vectors with weights using ward_distribution.


%{%}
for a = alpha
    b = a
    %for b = beta
        for r = rho
            hold off
            nexttile
            
            draw_axis();
            vectarrow(V.*2, [0 0 0]);
            hold on
            axis equal
            
            draw_ward_vectors(V, N, i_ang_theta, i_ang_phi, a, b, r);
            % title("alpha:" + a + ", beta" + b + "");
            title("alpha:" + a + ", beta:" + b + ", rho: " + r + "");
        end
    %end
end

hold off






